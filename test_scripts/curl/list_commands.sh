#!/bin/zsh

webtoken=`./login.sh | jq -r '.access_token'`

curl -X GET "https://localhost:16001/api/v1/commands" \
  -H  "accept: application/json" \
  -H "Authorization: Bearer $webtoken" \
  --insecure

