import json  # alt. ijson
from pathlib import Path

database = Path("/var/db/sensor.json")
builtin = {
    "%": {"sensor": "PCB", "label": "Station health"},
}


def catalogue(key=None):
    with database.open() as f:
        return json.load(f)
