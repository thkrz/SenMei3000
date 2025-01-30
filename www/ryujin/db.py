import json  # alt. ijson
import re
from collections import defaultdict
from pathlib import Path

db_root = Path("./db")
db_stat = db_root / "station"
db_stab = db_root / "sensor.tab"


def sadd(o):
    # o = {
    #     "name": "",
    #     "schema": [
    #         {"var": "", "unit": "", "idx": 0},
    #         {"var": "", "unit": "", "idx": 0},
    #     ],
    # }
    if db_stab.exists():
        with open(db_stab) as f:
            j = json.load(f)
    else:
        j = []
    j.append(o)
    with open(db_stab, "w") as f:
        json.dump(j, f)


def _stab():
    with open(db_stab) as f:
        j = json.load(f)
    return j


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
    t = []
    s = defaultdict(list)
    eof = False
    i = 0
    while i < len(ln) - 4 and not eof:
        t.append(ln[i])
        s["#"].append(float(ln[i + 1]))
        s["*"].append([float(ln[i + 2]), float(ln[i + 3])])
        i += 4
        eof = True
        for sen in ln[i:]:
            i += 1
            if not sen:
                eof = False
                break
            j, d = _lex(sen)
            s[j].append(d)
    return t, s


def insert(sid, item):
    f = db_stat / sid
    t, s = _parse(item)

    if not f.exists():
        o = {
            "id": sid,
            "name": "",
            "lat": 0.0,
            "lng": 0.0,
            "comment": "",
            "sensor": {k: {"type": -1, "label": ""} for k in s.keys() if k.isalnum()},
        }
        o["t"] = t
        o["s"] = s
    else:
        with open(f) as ifd:
            o = json.load(ifd)
        o["t"] += t
        for k in s.keys():
            o["s"][k] += s[k]

    with open(f, "w") as ofd:
        json.dump(o, ofd)


def list():
    r = []
    for f in db_stat.iterdir():
        with open(f) as fd:
            o = json.load(fd)
        del o["t"]
        del o["s"]
        r.append(o)
    return r


def select(sid, include_tab=True):
    f = db_stat / sid
    with open(f) as fd:
        o = json.load(fd)
    if include_tab:
        return {"dat": o, "tab": _stab()}
    return o


def update(sid, **kwargs):
    assert all([k not in kwargs.keys() for k in ["id", "t", "s"]])
    o = select(sid, include_tab=False)
    o.update(kwargs)
    with open(db_stat / sid, "w") as ofd:
        json.dump(o, ofd)
