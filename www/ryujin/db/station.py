import json  # alt. ijson
import numpy as np
import re
from collections import defaultdict
from datetime import datetime
from pathlib import Path

database = Path("./db/station")


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
        x["temp"].append(float(ln[i + 2]))
        x["rh"].append(float(ln[i + 3]))
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
        with open(f / "meta.json") as ifd:
            o = json.load(ifd)
        r.append(o)
    return r


def insert(sid, item):
    root = database / sid
    if not root.exists():
        Path.mkdir(root)
    with open(root / "raw.txt", "a") as ofd:
        ofd.write(item + "\r\n")

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
        with open(p, "w") as ofd:
            json.dump(m, ofd)

    for k, v in x.items():
        with open(root / (k + ".csv"), "a") as ofd:
            np.savetxt(ofd, v, delimiter=",", fmt="%.4f")


def select(sid):
    p = database / sid
    with open(p / "meta.json") as ifd:
        a = json.load(ifd)
    b = {f.stem: np.loadtxt(f, delimiter=",") for f in p.glob("*.csv")}
    return a, b


def update(sid, **kwargs):
    mf = database / sid / "meta.json"
    with open(mf) as ifd:
        o = json.load(ifd)
    _upd(o, kwargs, exclude=["id"])
    with open(mf, "w") as ofd:
        json.dump(o, ofd)
