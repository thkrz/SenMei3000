import re


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


class Data:
    def load(self, name):
        with open(name) as f:
            return self.loads(f.read())

    def loads(self, s):
        tok = s.split()
        dt = tok[0]
        bat = float(tok[1])
        temp = float(tok[2])
        rh = float(tok[3])
        rd = {}
        for sen in tok[4:]:
            i, d = _lex(sen)
            rd[i] = d
        return {"DATE": dt, "BAT": bat, "TEMP": temp, "RH": rh, "MEAS": rd}
