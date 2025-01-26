#!/bin/bash
data() {
  cat<<EOF
2025-01-26
11.75
a+23.54-7.8
0+23.54-7.8
Z+23.54-7.8
EOF
}
curl -X POST -H "Content-Type: text/plain" --data-binary @data.txt "http://127.0.0.1:8000/station/1"
