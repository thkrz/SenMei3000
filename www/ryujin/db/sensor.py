import json  # alt. ijson
from pathlib import Path

database = Path("./db/sensor.tab")


def insert(o):
    # o = {
    #     "name": "",
    #     "schema": [
    #         {"var": "", "unit": "", "idx": 0},
    #         {"var": "", "unit": "", "idx": 0},
    #     ],
    # }
    if database.exists():
        with open(database) as f:
            j = json.load(f)
    else:
        j = []
    j.append(o)
    with open(database, "w") as f:
        json.dump(j, f)


def list():
    with open(database) as f:
        j = json.load(f)
    return j
