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


async def catalogue(request):
    return JSONResponse(db.station.catalogue())


async def download(request):
    sid = request.path_params["sid"]
    k = request.path_params["k"]
    meta, arr = db.station.select(sid, key=k)
    cfg = meta["config"]
    schema = db.sensor.catalogue()
    if k in cfg.keys():
        nam = cfg[k]["sensor"]
    else:
        nam = "PCB"
    hdr = ",".join(["Time"] + schema[nam])
    name = f"{sid}_{k}.csv"
    path = "data/" + name
    np.savetxt(path, arr, delimiter=",", fmt="%.3f", header=hdr.upper())
    return FileResponse(path, filename=name)


async def prepare(data, config):
    schema = db.sensor.catalogue()
    s = {}
    for k, cfg in config.items():
        nam = cfg["sensor"]
        if nam not in schema.keys():
            s[k] = errmsg(f"{k}: sensor {nam} is not configured yet")
            continue
        if k not in data.keys():
            s[k] = errmsg(f"{k}: no data available")
            continue
        y = data[k]
        if len(y.shape) == 1:
            y = y[:, None]
        labels = schema[nam]
        title = f"Sensor: {k}\u00A0({nam})"
        lbl = cfg["label"]
        if lbl:
            title += "\u00A0/ " + lbl
        s[k] = {
            "data": y.tolist(),
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


async def station(request):
    sid = request.path_params["sid"]
    if request.method == "POST":
        b = await request.body()
        try:
            db.station.insert(sid, b.decode("utf-8"))
        except Exception as ex:
            return PlainTextResponse(str(ex) + "\r\n", status_code=501)
        return PlainTextResponse("success.\r\n", status_code=201)
    meta, data_ = db.station.select(sid)
    data = {"time": [], "series": [], "health": []}
    if len(data_) > 0:
        data["time"] = data_["time"].tolist()
        data["series"] = await prepare(data_, meta["config"])
        data["health"] = await prepare(data_, db.sensor.builtin)
    return JSONResponse({"meta": meta, "data": data})


async def update(request):
    sid = request.path_params["sid"]
    o = await request.json()
    db.station.update(sid, **o)
    return PlainTextResponse("success.\r\n", status_code=201)


routes = [
    Route("/sensor", sensor, methods=["GET", "POST"]),
    Route("/station/{sid}/{k}/download", download, methods=["GET"]),
    Route("/station/{sid}/update", update, methods=["POST"]),
    Route("/station/{sid}", station, methods=["GET", "POST"]),
    Route("/station", catalogue, methods=["GET"]),
    # DEBUG
    Mount("/", app=StaticFiles(directory="html", html=True), name="static"),
]
app = Starlette(routes=routes)

if __name__ == "__main__":
    # uvicorn.run("hoori:app", host="0.0.0.0", log_level="info")
    uvicorn.run("hoori:app", host="127.0.0.1", log_level="info")
