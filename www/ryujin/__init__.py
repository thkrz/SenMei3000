import numpy as np
import uvicorn
from starlette.applications import Starlette
from starlette.responses import PlainTextResponse, JSONResponse
from starlette.routing import Route

# DEBUG
from starlette.routing import Mount
from starlette.staticfiles import StaticFiles

from . import db


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

    o = db.station.select(sid)
    sen = db.sensor.list()
    data = o["data"]
    del o["data"]
    s = {}
    for k in o["config"].keys():
        i = o["config"][k]["sensor"]
        y = np.array(data[k])
        idx = []
        labels = []
        for col in sen[i]["schema"]:
            idx.append(col["idx"])
            labels.append(f"{col['var']} [{col['unit']}]")
        a = y[:, tuple(idx)].tolist()
        s[k] = {
            "data": a,
            "labels": labels,
            "length": len(labels),
            "title": f"Sensor {k} ({sen[i]['name']})<br />{o['config'][k]['label']}",
        }
    o["t"] = data["!"]
    o["s"] = s
    o["h"] = {
        0: {
            "data": data["#"],
            "labels": ["Battery [V]"],
            "length": 1,
            "title": "Battery Voltage",
        },
        1: {
            "data": data["*"],
            "labels": ["Temperature [°C]", "Humidity [-]"],
            "length": 2,
            "title": "Station Climate",
        },
    }
    return JSONResponse(o)


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
