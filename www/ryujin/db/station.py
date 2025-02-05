import json  # alt. ijson
import numpy as np
import re
from collections import defaultdict
from datetime import datetime
from pathlib import Path

database = Path("./db/station")


def _dump(p, a, idx=None):
    a = np.asarray(a)
    if p.exists():
        b = np.load(p)
        a = np.concatenate((b, a), axis=0)
    if idx is None:
        idx = a.argsort()
    with p.open("wb") as fop:
        np.save(fop, a[idx])
    return idx


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
    x = defaultdict(list)
    cfg = {}
    rows = iter(s.splitlines())
    for ln in rows:
        if ln == "CALIBRATE":
            for ln in rows:
                if not ln:
                    break
                k, sens = ln[0], ln[11:17]
                cfg[k] = {"sensor": sens, "label": ""}
        else:
            dt = datetime.fromisoformat(ln)
            x["time"].append(dt.timestamp())  # must be first key in dict
            x["bat"].append(float(next(rows)))
            x["sht"].append([float(next(rows)), float(next(rows))])
            for ln in rows:
                if not ln:
                    break
                k, n = _lex(ln)
                x[k].append(n)
    return cfg, x


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
        root.mkdir()

    cfg, x = _parse(item)
    p = root / "meta.json"
    if not p.exists():
        assert len(cfg) > 0
        m = {
            "id": sid,
            "name": "",
            "lat": 0.0,
            "lng": 0.0,
            "config": cfg,
        }
        with p.open("w") as fod:
            json.dump(m, fod)
    elif len(cfg) > 0:
        with p.open() as fid:
            m = json.load(fid)
        now = datetime.now().timestamp()
        r = root / str(int(now))
        rebase = False
        for k, v in m["config"].items():
            if k not in cfg.keys() or v["sensor"] != cfg[k]["sensor"]:
                r.mkdir(exists_ok=True)
                q = root / (k + ".bin")
                q.rename(r / (k + ".bin"))
                rebase = True
        if rebase:
            q = r / "meta.json"
            with q.open("w") as fod:
                json.dump(m, fod)
        m["config"] = cfg
        with p.open("w") as fod:
            json.dump(m, fod)

    idx = None
    for k, v in x.items():
        idx = _dump(root / (k + ".bin"), v, idx)


def select(sid, key=None):
    p = database / sid
    with open(p / "meta.json") as fid:
        a = json.load(fid)
    if key:
        t = np.load(p / "time.bin")
        b = np.load(p / (key + ".bin"))
        b = np.column_stack((t, b))
    else:
        b = {f.stem: np.load(f) for f in p.glob("*.bin")}
    return a, b


def update(sid, **kwargs):
    mp = database / sid / "meta.json"
    with mp.open() as fid:
        o = json.load(fid)
    _upd(o, kwargs, exclude=["id"])
    with mp.open("w") as fod:
        json.dump(o, fod)
