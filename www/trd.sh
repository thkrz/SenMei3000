#!/bin/bash
payload="$1"
curl -X POST -H "Content-Type: text/plain" --data-binary "@$1" "http://127.0.0.1:8000/station/1"
