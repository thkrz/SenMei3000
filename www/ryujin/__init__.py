import uvicorn
from starlette.applications import Starlette
from starlette.responses import PlainTextResponse, JSONResponse
from starlette.routing import Route

from . import sdi12


async def station(request):
    sid = request.path_params["sid"]
    if request.method == "POST":
        b = await request.body()
        o = sdi12.loads(b.decode("utf-8"))
        print(sid)
        print(o)
        return PlainTextResponse("data inserted\r\n", status_code=201)
    return JSONResponse({"Hello": "World!"})


routes = [Route("/station/{sid}", station, methods=["GET", "POST"])]
app = Starlette(routes=routes)

if __name__ == "__main__":
    # uvicorn.run("ryujin:app", host="0.0.0.0", log_level="info")
    uvicorn.run("ryujin:app", host="127.0.0.1", log_level="info")
