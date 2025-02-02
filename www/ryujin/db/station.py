import json  # alt. ijson
import re
from collections import defaultdict
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
    s = defaultdict(list)
    eof = False
    i = 0
    while i < len(ln) - 4 and not eof:
        s["#time"].append(ln[i])
        s["#volt"].append(float(ln[i + 1]))
        s["#clim"].append([float(ln[i + 2]), float(ln[i + 3])])
        i += 4
        eof = True
        for sen in ln[i:]:
            i += 1
            if not sen:
                eof = False
                break
            j, d = _lex(sen)
            s[j].append(d)
    return s


def _upd(o, n, exclude=[]):
    for k, v in n.items():
        if k in exclude:
            continue
        if isinstance(v, dict):
            assert isinstance(o[k], dict)
            _upd(o[k], v)
        else:
            o[k] = v


def insert(sid, item):
    with open(database / (sid + ".raw"), "a") as f:
        f.write(item + "\r\n")

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
        d = defaultdict(list)
    else:
        with open(df) as ifd:
            d = json.load(ifd)

    for k in s.keys():
        d[k] += s[k]
    with open(df, "w") as ofd:
        json.dump(d, ofd)


def list():
    r = []
    for f in database.glob("*.meta"):
        with open(f) as fd:
            o = json.load(fd)
        r.append(o)
    return r


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
