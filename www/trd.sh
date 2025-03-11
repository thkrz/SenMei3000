#!/bin/bash
curl -X POST -H "Content-Type: text/plain" --data-binary @data.txt -i "http://erdrutsch.com:8000/station/LAB2"
