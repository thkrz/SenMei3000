CREATE TABLE sensors (
  id     INTEGER PRIMARY KEY,
  name   TEXT NOT NULL,
  params TEXT NOT NULL
);

CREATE TABLE stations (
  id   INTEGER PRIMARY KEY,
  name TEXT NOT NULL,
  meta TEXT NOT NULL
);

CREATE TABLE data (
  id      INTEGER PRIMARY KEY,
  name    TEXT NOT NULL,
  time    INTEGER NOT NULL,
  reading TEXT NOT NULL,
  UNIQUE(name, time) ON CONFLICT REPLACE
);

