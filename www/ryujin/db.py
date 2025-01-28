import json
import re
from pathlib import Path

database = Path("./db/station")


def _num(s):
    try:
        n = int(s)
    except ValueError:
        n = float(s)
    return n


def _lex(s):
    idx, s = s[0], s[1:]
    i = 0
    n = []
    for m in re.finditer(r"[+-]", s):
        j = m.start(0)
        if j == 0:
            continue
        n.append(_num(s[i:j]))
        i = j
    n.append(_num(s[i:]))
    return idx, n


def _parse(s):
    ln = s.splitlines()
    rd = []
    eof = False
    i = 0
    while i < len(ln) - 4 and not eof:
        dt = ln[i]
        bat = float(ln[i + 1])
        temp = float(ln[i + 2])
        rh = float(ln[i + 3])
        data = {}
        eof = True
        i += 4
        for sen in ln[i:]:
            i += 1
            if not sen:
                eof = False
                break
            j, d = _lex(sen)
            data[j] = d
        rd.append({"date": dt, "bat": bat, "temp": temp, "rh": rh, "data": data})
    return rd


def insert(sid, item):
    f = database / sid
    rd = _parse(item)

    if not f.exists():
        o = {
            "id": sid,
            "name": "",
            "lat": 0.0,
            "lng": 0.0,
            "comment": "",
            "timeseries": [],
        }
        schema = {}
        for k in rd[0]["data"].keys():
            schema[k] = {"var": "", "unit": "", "label": ""}
        o["schema"] = schema
    else:
        with open(f) as ifd:
            o = json.load(ifd)

    o["timeseries"].append(rd)

    with open(f, "w") as ofd:
        json.dump(o, ofd)


def list():
    r = []
    for f in database.iterdir():
        with open(f) as fd:
            o = json.load(fd)
        del o["timeseries"]
        r.append(o)
    return r


def select(sid):
    f = database / sid
    with open(f) as fd:
        o = json.load(fd)
    return o


def update(sid, **kwargs):
    assert all([k not in kwargs.keys() for k in ["id", "timeseries"]])
    o = select(sid)
    o.update(kwargs)
    with open(database / sid, "w") as ofd:
        json.dump(o, ofd)
