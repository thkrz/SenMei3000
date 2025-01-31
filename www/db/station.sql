CREATE TABLE sensor (
    id serial PRIMARY KEY,
    name bpchar,
    parameter bpchar[][]
);

CREATE TABLE station (
    id bpchar PRIMARY KEY,
    latlng point,
    name bpchar,
    note bpchar
);

CREATE TABLE data (
    station_id bpchar,
    sensor_id integer,
    datetime timestamp,
    value real array
);

CREATE TABLE config (
    id char(1),
    sensor_id integer,
);

