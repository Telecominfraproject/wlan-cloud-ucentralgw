#!/bin/bash

webtoken=`./login.sh | jq -r '.access_token'`

curl -X GET "https://localhost:16001/api/v1/devices" \
  -H  "accept: application/json" \
  -H "Authorization: Bearer $webtoken" \
  --insecure

