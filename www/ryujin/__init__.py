import uvicorn
from starlette.applications import Starlette
from starlette.responses import PlainTextResponse, JSONResponse
from starlette.routing import Route

# DEBUG
from starlette.routing import Mount
from starlette.staticfiles import StaticFiles

from . import db


async def station(request):
    sid = request.path_params["sid"]
    if request.method == "POST":
        b = await request.body()
        db.insert(sid, b.decode("utf-8"))
        return PlainTextResponse("data inserted\r\n", status_code=201)
    return JSONResponse(db.select(sid))


async def station_list(request):
    return JSONResponse(db.list())


routes = [
    Route("/station/{sid}", station, methods=["GET", "POST"]),
    Route("/station", station_list, methods=["GET"]),
    # DEBUG
    Mount("/", app=StaticFiles(directory="html", html=True), name="static"),
]
app = Starlette(routes=routes)

if __name__ == "__main__":
    # uvicorn.run("ryujin:app", host="0.0.0.0", log_level="info")
    uvicorn.run("ryujin:app", host="127.0.0.1", log_level="info")
