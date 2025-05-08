import numpy as np
import uvicorn
from starlette.applications import Starlette
from starlette.responses import PlainTextResponse, JSONResponse, FileResponse
from starlette.routing import Route
from starlette.templating import Jinja2Templates

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
    meta = db.station.select(sid, stats=False)
    label = meta["config"][k]["label"]
    name = f"{sid}_"
    name += label if label else k
    name += ".csv"
    path = f"/var/db/station/{sid}/{k}.csv"
    return FileResponse(path, filename=name)


async def sensor(request):
    if request.method == "POST":
        o = await request.json()
        db.sensor.update(o)
        return PlainTextResponse("success.\r\n", status_code=201)
    return JSONResponse(db.sensor.catalogue())


async def station(request):
    sid = request.path_params["sid"]
    if request.method == "POST":
        try:
            b = await request.body()
            lines = b.decode("utf-8").strip().splitlines()
            if lines[0] == "CONFIG":
                db.station.config(sid, lines[1:])
            else:
                db.station.insert(sid, lines)
        except Exception as ex:
            errmsg = f"{__file__}: Line {ex.__traceback__.tb_lineno}: {type(ex).__name__}, {ex}\r\n"
            return PlainTextResponse(errmsg, status_code=501)
        return PlainTextResponse("success.\r\n", status_code=201)
    return templates.TemplateResponse(request, "station.html", db.station.select(sid))


async def svg(request):
    sid = request.path_params["sid"]
    k = request.path_params["k"]
    name = f"{k}.svg"
    path = f"/var/db/station/{sid}/{k}.svg"
    return FileResponse(path, filename=name)


async def update(request):
    sid = request.path_params["sid"]
    o = await request.json()
    db.station.update(sid, **o)
    return PlainTextResponse("success.\r\n", status_code=201)


templates = Jinja2Templates(directory="html/templates")
routes = [
    Route("/sensor", sensor, methods=["GET", "POST"]),
    Route("/station/svg/{sid}/{k}", svg, methods=["GET"]),
    Route("/station/{sid}/{k}", download, methods=["GET"]),
    Route("/station/{sid}/update", update, methods=["POST"]),
    Route("/station/{sid}", station, methods=["GET", "POST"]),
    Route("/station", catalogue, methods=["GET"]),
    # DEBUG
    Mount("/", app=StaticFiles(directory="html", html=True), name="static"),
]
app = Starlette(routes=routes)

if __name__ == "__main__":
    # uvicorn.run("hoori:app", uds="/tmp/hoori.socket", log_level="info")
    # uvicorn.run("hoori:app", host="127.0.0.1", log_level="info")
    pass
