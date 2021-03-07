#!/bin/zsh

serial=$1

webtoken=`./login.sh | jq -r '.access_token'`

curl -X GET "https://localhost:16001/api/v1/device/$serial/status" \
  -H  "accept: application/json" \
  -H "Authorization: Bearer $webtoken" \
  --insecure
