#!/bin/bash

webtoken=`./login.sh | jq -r '.access_token'`

curl -X GET "https://ucentral.dpaas.arilia.com:16001/api/v1/commands" \
  -H  "accept: application/json" \
  -H "Authorization: Bearer $webtoken" \

