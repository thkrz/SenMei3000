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
    y = defaultdict(list)
    eof = False
    i = 0
    keys = []
    while i < len(ln) - 4 and not eof:
        dt = datetime.fromisoformat(ln[i])
        t = dt.timestamp()
        y["bat"].append([t, float(ln[i + 1])])
        y["env"].append([t, float(ln[i + 2]), float(ln[i + 3])])
        i += 4
        eof = True
        for rd in ln[i:]:
            i += 1
            if not rd:
                eof = False
                break
            k, n = _lex(rd)
            y[k].append([t] + n)
            keys.append(k)
    return keys, y


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
    root = database / sid
    if not root.exists():
        Path.mkdir(root)
    with open(root / "raw", "a") as ofd:
        ofd.write(item + "\r\n")

    keys, y = _parse(item)
    p = root / "meta"
    if not p.exists():
        m = {
            "id": sid,
            "name": "",
            "lat": 0.0,
            "lng": 0.0,
            "comment": "",
            "config": {k: {"sensor": -1, "label": ""} for k in keys},
        }
        with open(p, "w") as ofd:
            json.dump(m, ofd)
    for k, v in y.items():
        with open(root / k, "a") as ofd:
            for i in range(len(v)):
                ofd.write(",".join([str(n) for n in v[i]]) + "\n")


def select(sid, include_data=True):
    mf = database / sid / "meta"
    with open(mf) as ifd:
        o = json.load(ifd)
    if include_data:
        df = database / sid
        d = {}
        for p in df.iterdir():
            k = p.name
            if len(k) != 1:
                continue
            d[k] = np.loadtxt(p, delimiter=",").tolist()
        return o, d
    return o


def update(sid, **kwargs):
    o = select(sid, include_data=False)
    _upd(o, kwargs, exclude=["id"])
    with open(database / (sid + ".meta"), "w") as ofd:
        json.dump(o, ofd)
