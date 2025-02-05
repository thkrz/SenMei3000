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


async def prepare(meta, data, schema=None):
    if schema is None:
        schema = meta["schema"]
    s = {}
    for k, cfg in meta["config"].items():
        name = cfg["sensor"]
        y = data[k]
        idx = tuple(schema[name]["idx"])
        param = schema[name]["parameter"]
        a = y[:, idx] if len(y.shape) > 1 else y
        labels = [param[j] for j in idx]
        title = f"Sensor: {k}\u00A0({name})"
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
    meta, data = db.station.select(sid)
    schema = db.sensor.catalogue()
    return JSONResponse(
        {
            "meta": meta,
            "data": {
                "time": data["time"].tolist(),
                "series": await prepare(meta, data, schema),
                "health": await prepare(db.sensor.builtin, data),
            },
        }
    )


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
