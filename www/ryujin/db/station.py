import json  # alt. ijson
import re
from collections import defaultdict
from pathlib import Path

raw = Path("./db/station.raw")
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
        s["!"].append(ln[i])
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
    return s


def insert(sid, item):
    with open(raw, "a") as f:
        f.write(item + "\r\n")
    f = database / sid
    s = _parse(item)

    if not f.exists():
        o = {
            "id": sid,
            "name": "",
            "lat": 0.0,
            "lng": 0.0,
            "comment": "",
            "config": {k: {"sensor": -1, "label": ""} for k in s.keys() if k.isalnum()},
            "data": s,
        }
    else:
        with open(f) as ifd:
            o = json.load(ifd)
        for k in s.keys():
            o["s"][k] += s[k]

    with open(f, "w") as ofd:
        json.dump(o, ofd)


def list():
    r = []
    for f in database.iterdir():
        with open(f) as fd:
            o = json.load(fd)
        del o["data"]
        r.append(o)
    return r


def select(sid):
    f = database / sid
    with open(f) as fd:
        o = json.load(fd)
    return o


def update(sid, **kwargs):
    assert all([k not in kwargs.keys() for k in ["id", "data"]])
    o = select(sid)
    cfg = kwargs.get("config")
    if cfg is not None:
        del kwargs["config"]
        for k, v in cfg.items():
            o["config"][k].update(v)
    o.update(kwargs)
    with open(database / sid, "w") as ofd:
        json.dump(o, ofd)
