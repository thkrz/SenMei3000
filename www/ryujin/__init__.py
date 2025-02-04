import numpy as np
import tempfile
import uvicorn
from starlette.applications import Starlette
from starlette.responses import PlainTextResponse, JSONResponse
from starlette.routing import Route

# DEBUG
from starlette.routing import Mount
from starlette.staticfiles import StaticFiles

from . import db


async def download(request):
    sid = request.path_params["sid"]
    k = request.path_params["k"]
    arr = db.station.select(sid, key=k)
    with tempfile.NamedTemporaryFile(delete_on_close=False) as fp:
        np.savetxt(fp, arr[arr[:, 0].argsort()], delimiter=",", fmt="%.4f")
        fp.close()
        with open(fp.name) as f:
            s = f.read()
        return PlainTextResponse(s)


async def prepare(meta, inds, data, schema=None):
    if schema is None:
        schema = meta["schema"]
    s = {}
    for k, cfg in meta["config"].items():
        i = cfg["sensor"]
        if i < 0:
            s[k] = None
            continue
        y = data[k]
        idx = schema[i]["idx"]
        labels = schema[i]["parameter"]
        if isinstance(idx, list):
            a = y[:, tuple(idx)]
        else:
            a = y
        title = f"Sensor: {k}\u00A0({schema[i]['name']})"
        lbl = cfg["label"]
        if lbl:
            title += "\u00A0/ " + lbl
        s[k] = {
            "data": a[inds].tolist(),
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
        db.station.insert(sid, b.decode("utf-8"))
        return PlainTextResponse("success.\r\n", status_code=201)
    meta, data = db.station.select(sid)
    schema = db.sensor.catalogue()
    inds = data["time"].argsort()
    t = data["time"][inds]
    return JSONResponse(
        {
            "schema": schema,
            "meta": meta,
            "data": {
                "time": t.tolist(),
                "series": await prepare(meta, inds, data, schema),
                "health": await prepare(db.sensor.builtin, inds, data),
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
