import json  # alt. ijson
import re
from collections import defaultdict
# from datetime import datetime
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
    d = defaultdict(list)
    eof = False
    i = 0
    while i < len(ln) - 4 and not eof:
        # dt = datetime.fromisoformat(ln[i])
        # d["#time"].append(dt.timestamp())
        d["#time"].append(ln[i])
        d["#volt"].append(float(ln[i + 1]))
        d["#clim"].append([float(ln[i + 2]), float(ln[i + 3])])
        i += 4
        eof = True
        for rd in ln[i:]:
            i += 1
            if not rd:
                eof = False
                break
            k, n = _lex(rd)
            d[k].append(n)
    return d


def _upd(o, n, exclude=[]):
    for k, v in n.items():
        if k in exclude:
            continue
        if isinstance(v, dict):
            assert isinstance(o[k], dict)
            _upd(o[k], v)
        else:
            o[k] = v


def catalogue():
    r = []
    for f in database.glob("*.meta"):
        with open(f) as ifd:
            o = json.load(ifd)
        r.append(o)
    return r


def insert(sid, item):
    with open(database / (sid + ".raw"), "a") as ofd:
        ofd.write(item + "\r\n")

    mf = database / (sid + ".meta")
    df = database / (sid + ".dat")

    s = _parse(item)

    if not mf.exists():
        m = {
            "id": sid,
            "name": "",
            "lat": 0.0,
            "lng": 0.0,
            "comment": "",
            "config": {k: {"sensor": -1, "label": ""} for k in s.keys() if k.isalnum()},
        }
        with open(mf, "w") as ofd:
            json.dump(m, ofd)
        del m
    if not df.exists():
        d = defaultdict(list)
    else:
        with open(df) as ifd:
            d = json.load(ifd)

    for k in s.keys():
        d[k] += s[k]
    with open(df, "w") as ofd:
        json.dump(d, ofd)


def select(sid, include_data=True):
    mf = database / (sid + ".meta")
    with open(mf) as ifd:
        o = json.load(ifd)
    if include_data:
        df = database / (sid + ".dat")
        with open(df) as ifd:
            j = json.load(ifd)
        return o, j
    return o


def update(sid, **kwargs):
    o = select(sid, include_data=False)
    _upd(o, kwargs, exclude=["id"])
    with open(database / (sid + ".meta"), "w") as ofd:
        json.dump(o, ofd)
