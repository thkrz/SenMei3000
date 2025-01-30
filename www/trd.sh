#!/bin/bash
curl -X POST -H "Content-Type: text/plain" --data-binary @data.txt "http://127.0.0.1:8000/station/1"
