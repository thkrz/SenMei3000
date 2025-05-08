#!/bin/bash
n=$1
for data in post/*.txt; do
  curl -X POST -H "Content-Type: text/plain" --data-binary "@${data}" -i "http://erdrutsch.com:8000/station/TEST1"
  n=$((n-1))
  if [ $n -eq 0 ]; then
    break
  fi
done
