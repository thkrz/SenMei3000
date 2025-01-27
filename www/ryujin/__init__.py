import re
import uvicorn
from starlette.applications import Starlette
from starlette.responses import PlainTextResponse, JSONResponse
from starlette.routing import Route


def num(s):
    return float(s) if "." in s else int(s)


def lex(s):
    idx, s = s[0], s[1:]
    i = 0
    n = []
    for m in re.finditer(r"[+-]", s):
        j = m.start(0)
        if j == 0:
            continue
        n.append(num(s[i:j]))
        i = j
    n.append(num(s[i:]))
    return idx, n


def parse(s):
    tok = s.split()
    dt = tok[0].strip()
    bat = float(tok[1])
    temp = float(tok[2])
    rh = float(tok[3])
    rd = {}
    for sen in tok[4:]:
        i, d = lex(sen)
        rd[i] = d
    return {"DATE": dt, "BAT": bat, "TEMP": temp, "RH": rh, "RD": rd}


async def station(request):
    sid = request.path_params["sid"]
    if request.method == "POST":
        b = await request.body()
        v = parse(b.decode("utf-8"))
        print(sid)
        print(v)
        return PlainTextResponse("data inserted\r\n", status_code=201)
    return JSONResponse({"Hello": "World!"})


routes = [Route("/station/{sid}", station, methods=["GET", "POST"])]
app = Starlette(routes=routes)

if __name__ == "__main__":
    # uvicorn.run("ryujin:app", host="0.0.0.0", log_level="info")
    uvicorn.run("ryujin:app", host="127.0.0.1", log_level="info")
