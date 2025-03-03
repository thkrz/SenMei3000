#!/bin/bash
curl -X POST -H "Content-Type: text/plain" --data-binary @data.txt "http://erdrutsch.com:8000/station/LAB2" \
  --trace-ascii -
