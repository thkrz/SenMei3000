import json  # alt. ijson
from pathlib import Path

database = Path("./db/sensor.tab")
builtin = (
    {
        "config": {
            "bat": {"sensor": "BAT", "label": "Battery"},
            "sht": {"sensor": "SHTC3", "label": "Station climate"},
        }
    },
    {
        "BAT": {"idx": [0], "parameter": ["Voltage\u00A0[V]"]},
        "SHTC3": {
            "idx": [0, 1],
            "parameter": ["Temperature\u00A0[°C]", "Humidity\u00A0[-]"],
        },
    },
)


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
