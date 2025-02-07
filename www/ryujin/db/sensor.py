import json  # alt. ijson
from pathlib import Path

database = Path("./db/sensor.json")
builtin = {
    "bat": {"sensor": "BAT", "label": "Battery"},
    "sht": {"sensor": "SHTC3", "label": "Station climate"},
}


def insert(o):
    if database.exists():
        with database.open() as f:
            j = json.load(f)
    else:
        j = []
    j.append(o)
    with database.open("w") as f:
        json.dump(j, f)


def catalogue():
    with database.open() as f:
        return json.load(f)
