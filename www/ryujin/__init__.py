import numpy as np
import uvicorn
from starlette.applications import Starlette
from starlette.responses import PlainTextResponse, JSONResponse, FileResponse
from starlette.routing import Route

# DEBUG
from starlette.routing import Mount
from starlette.staticfiles import StaticFiles

from . import db


def errmsg(s):
    return {"length": 0, "title": s}


async def download(request):
    sid = request.path_params["sid"]
    k = request.path_params["k"]
    meta, arr = db.station.select(sid, key=k)
    schema = db.sensor.catalogue()
    i = meta["config"][k]["sensor"]
    hdr = ",".join(["Time"] + schema[i]["parameter"])
    name = f"{sid}_{k}.csv"
    path = "data/" + name
    np.savetxt(path, arr, delimiter=",", fmt="%.3f", header=hdr.upper())
    return FileResponse(path, filename=name)


async def prepare(data, meta, schema):
    s = {}
    for k, cfg in meta["config"].items():
        nam = cfg["sensor"]
        if nam not in schema.keys():
            s[k] = errmsg(f"{k}: sensor {nam} is not configured yet")
            continue
        y = data[k]
        dim = len(y.shape)
        idx = tuple(schema[nam]["idx"])
        param = schema[nam]["parameter"]
        if max(idx) >= dim:
            s[k] = errmsg(f"{k}: sensor {nam} missmatch")
            continue
        a = y[:, idx] if dim > 1 else y
        labels = [param[j] for j in idx]
        title = f"Sensor: {k}\u00A0({nam})"
        lbl = cfg["label"]
        if lbl:
            title += "\u00A0/ " + lbl
        s[k] = {
            "data": a.tolist(),
            "labels": labels,
            "length": len(labels),
            "title": title,
        }
    return s


async def sensor(request):
    if request.method == "POST":
        o = await request.json()
        db.sensor.update(o)
        return PlainTextResponse("success.\r\n", status_code=201)
    return JSONResponse(db.sensor.catalogue())


async def update(request):
    sid = request.path_params["sid"]
    o = await request.json()
    db.station.update(sid, **o)
    return PlainTextResponse("success.\r\n", status_code=201)


async def station(request):
    try:
        sid = request.path_params["sid"]
    except KeyError:
        return JSONResponse(db.station.catalogue())
    if request.method == "POST":
        b = await request.body()
        try:
            db.station.insert(sid, b.decode("utf-8"))
        except Exception as ex:
            return PlainTextResponse(str(ex) + "\r\n", status_code=401)
        return PlainTextResponse("success.\r\n", status_code=201)
    meta, data_ = db.station.select(sid)
    data = {"time": [], "series": [], "health": []}
    if len(data_) > 0:
        data["time"] = data_["time"].tolist()
        data["series"] = await prepare(data_, meta, db.sensor.catalogue())
        data["health"] = await prepare(data_, *db.sensor.builtin)
    return JSONResponse({"meta": meta, "data": data})


routes = [
    Route("/sensor", sensor, methods=["GET", "POST"]),
    Route("/station/{sid}/{k}/download", download, methods=["GET"]),
    Route("/station/{sid}/update", update, methods=["POST"]),
    Route("/station/{sid}", station, methods=["GET", "POST"]),
    Route("/station", station, methods=["GET"]),
    # DEBUG
    Mount("/", app=StaticFiles(directory="html", html=True), name="static"),
]
app = Starlette(routes=routes)

if __name__ == "__main__":
    # uvicorn.run("ryujin:app", host="0.0.0.0", log_level="info")
    uvicorn.run("ryujin:app", host="127.0.0.1", log_level="info")
