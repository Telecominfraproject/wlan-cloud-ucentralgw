#!/bin/zsh

serial=$1

curl -X GET "https://localhost:16001/api/v1/device/$serial" -H  "accept: application/json" --insecure
