import json  # alt. ijson
import numpy as np
import re
import string
from collections import defaultdict
from datetime import datetime
from pathlib import Path

from . import sensor
from .. import csv, svg

database = Path("/var/db/station")


def _touch(path, cols, exist_ok=False):
    if exist_ok or path.exists():
        return
    with open(path, "w") as fod:
        fod.write(",".join(["Time"] + cols) + "\n")


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


def _parse(rows):
    for ln in rows:
        yield _lex(ln)


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


def config(sid, rows):
    root = database / sid
    root.mkdir(exist_ok=True)

    cat = sensor.catalogue()
    cfg = sensor.builtin
    for s in rows:
        k, ident = s[0], s[11:17]
        cfg[k] = {"sensor": ident, "label": ""}
        _touch(root / f"{k}.csv", cat[ident])

    p = root / "meta.json"
    if not p.exists():
        m = {
            "id": sid,
            "name": "",
            "lat": 0.0,
            "lng": 0.0,
            "maintainer": "",
            "config": cfg,
        }
        with p.open("w") as fod:
            json.dump(m, fod)
        return
        # not reached

    with p.open() as fid:
        m = json.load(fid)
    for k, v in m["config"].items():
        f = root / f"{k}.csv"
        if k not in cfg.keys():
            f.unlink()
        elif v["sensor"] != cfg[k]["sensor"]:
            ident = cfg[k]["sensor"]
            _touch(f, cat[ident], exist_ok=True)
    m["config"] = cfg
    with p.open("w") as fod:
        json.dump(m, fod)


def insert(sid, rows):
    root = database / sid
    assert root.exists(), "database missing"
    # TODO:
    # validate and email alert
    t = rows[0]
    for k, a in _parse(rows[1:]):
        path = root / f"{k}.csv"
        with open(path, "a") as fid:
            fid.write(t + ",")
            fid.write(",".join([str(n) for n in a]) + "\n")
        svg.make(path)


def select(sid, stats=True):
    p = database / sid
    with open(p / "meta.json") as fid:
        o = json.load(fid)
    if stats:
        o["stats"] = {}
        for k in o["config"].keys():
            o["stats"][k] = csv.stats(p / f"{k}.csv")
    return o


def update(sid, **kwargs):
    mp = database / sid / "meta.json"
    with mp.open() as fid:
        o = json.load(fid)
    _upd(o, kwargs, exclude=["id"])
    with mp.open("w") as fod:
        json.dump(o, fod)
