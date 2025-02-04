import json  # alt. ijson
from pathlib import Path

database = Path("./db/sensor.tab")
builtin = {
    "config": {
        "bat": {"sensor": 0, "label": "Battery"},
        "sht": {"sensor": 1, "label": "Station climate"},
    },
    "schema": [
        {"name": "BAT", "idx": None, "parameter": ["Voltage\u00A0[V]"]},
        {
            "name": "SHTC3",
            "idx": [0, 1],
            "parameter": ["Temperature\u00A0[°C]", "Humidity\u00A0[-]"],
        },
    ],
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
