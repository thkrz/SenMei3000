import re
from datetime import datetime


def _num(s):
    return float(s) if "." in s else int(s)


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


def load(name):
    with open(name) as f:
        return loads(f.read())


def loads(s):
    tok = s.splitlines()
    rds = []
    eof = False
    i = 0
    while i < len(tok) - 4 and not eof:
        dt = datetime.fromisoformat(tok[i])
        bat = float(tok[i + 1])
        temp = float(tok[i + 2])
        rh = float(tok[i + 3])
        rd = {}
        eof = True
        i += 4
        for sen in tok[i:]:
            i += 1
            if not sen:
                eof = False
                break
            j, d = _lex(sen)
            rd[j] = d
        rds.append({"DATE": dt, "BAT": bat, "TEMP": temp, "RH": rh, "DATA": rd})
    return rds
