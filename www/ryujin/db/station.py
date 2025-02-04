import json  # alt. ijson
import numpy as np
import re
from collections import defaultdict
from datetime import datetime
from pathlib import Path

database = Path("./db/station")


def _dump(p, a):
    with p.open("a") as fod:
        np.savetxt(fod, a, delimiter=",", fmt="%.4f")


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


def _load(p):
    with p.open() as fid:
        return np.loadtxt(fid, delimiter=",")


def _num(s):
    try:
        n = int(s)
    except ValueError:
        n = float(s)
    return n


def _parse(s):
    ln = s.splitlines()
    x = defaultdict(list)
    i = 0
    while i < len(ln) - 4:
        dt = datetime.fromisoformat(ln[i])
        x["time"].append(dt.timestamp())
        x["bat"].append(float(ln[i + 1]))
        x["sht"].append([float(ln[i + 2]), float(ln[i + 3])])
        i += 4
        for rd in ln[i:]:
            i += 1
            if not rd:
                break
            k, n = _lex(rd)
            x[k].append(n)
    return x


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
    for f in database.iterdir():
        with open(f / "meta.json") as fid:
            o = json.load(fid)
        r.append(o)
    return r


def insert(sid, item):
    root = database / sid
    if not root.exists():
        Path.mkdir(root)
    with open(root / "raw.txt", "a") as fod:
        fod.write(item + "\r\n")

    x = _parse(item)
    p = root / "meta.json"
    if not p.exists():
        m = {
            "id": sid,
            "name": "",
            "lat": 0.0,
            "lng": 0.0,
            "comment": "",
            "config": {k: {"sensor": -1, "label": ""} for k in x.keys() if len(k) == 1},
        }
        with p.open("w") as fod:
            json.dump(m, fod)

    for k, v in x.items():
        _dump(root / (k + ".dat"), v)


def select(sid, key=None):
    p = database / sid

    if key:
        a = _load(p / "time.dat")
        b = _load(p / (key + ".dat"))
        return np.column_stack((a, b))

    with open(p / "meta.json") as fid:
        a = json.load(fid)
    b = {f.stem: _load(f) for f in p.glob("*.dat")}
    return a, b


def update(sid, **kwargs):
    mp = database / sid / "meta.json"
    with mp.open() as fid:
        o = json.load(fid)
    _upd(o, kwargs, exclude=["id"])
    with mp.open("w") as fod:
        json.dump(o, fod)
