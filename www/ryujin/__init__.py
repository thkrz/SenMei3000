import numpy as np
import uvicorn
from starlette.applications import Starlette
from starlette.responses import PlainTextResponse, JSONResponse
from starlette.routing import Route

# DEBUG
from starlette.routing import Mount
from starlette.staticfiles import StaticFiles

from . import db


async def prepare(meta, data, schema):
    s = {}
    for k in meta["config"].keys():
        i = meta["config"][k]["sensor"]
        if i < 0:
            s[k] = None
            continue
        y = np.array(data[k])
        idx = []
        labels = []
        for col in schema[i]["schema"]:
            idx.append(col["idx"])
            labels.append(f"{col['var']}\u00A0[{col['unit']}]")
        a = y[:, tuple(idx)].tolist()
        title = f"Sensor {k}: {schema[i]['name']}"
        lbl = meta["config"][k]["label"]
        if lbl:
            title += " / " + lbl
        s[k] = {"x": a, "labels": labels, "length": len(labels), "title": title}
    return s


async def sensor(request):
    if request.method == "POST":
        o = await request.json()
        db.sensor.update(o)
        return PlainTextResponse("success.\r\n", status_code=201)
    return JSONResponse(db.sensor.list())


async def stationupd(request):
    sid = request.path_params["sid"]
    o = await request.json()
    db.station.update(sid, **o)
    return PlainTextResponse("success.\r\n", status_code=201)


async def station(request):
    try:
        sid = request.path_params["sid"]
    except KeyError:
        return JSONResponse(db.station.list())
    if request.method == "POST":
        b = await request.body()
        db.station.insert(sid, b.decode("utf-8"))
        return PlainTextResponse("success.\r\n", status_code=201)

    meta, data = db.station.select(sid)
    schema = db.sensor.list()
    return JSONResponse(
        {
            "schema": schema,
            "meta": meta,
            "data": {
                "t": data["#time"],
                "s": await prepare(meta, data, schema),
                "r": {
                    0: {
                        "x": data["#volt"],
                        "labels": ["Voltage\u00A0[V]"],
                        "length": 1,
                        "title": "Battery",
                    },
                    1: {
                        "x": data["#clim"],
                        "labels": ["Temperature\u00A0[°C]", "Humidity\u00A0[-]"],
                        "length": 2,
                        "title": "Station Climate",
                    },
                },
            },
        }
    )


routes = [
    Route("/sensor", sensor, methods=["GET", "POST"]),
    Route("/station/{sid}/update", stationupd, methods=["POST"]),
    Route("/station/{sid}", station, methods=["GET", "POST"]),
    Route("/station", station, methods=["GET"]),
    # DEBUG
    Mount("/", app=StaticFiles(directory="html", html=True), name="static"),
]
app = Starlette(routes=routes)

if __name__ == "__main__":
    # uvicorn.run("ryujin:app", host="0.0.0.0", log_level="info")
    uvicorn.run("ryujin:app", host="127.0.0.1", log_level="info")
