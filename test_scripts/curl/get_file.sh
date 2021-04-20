#!/usr/bin/env bash

webtoken=`./login.sh | jq -r '.access_token'`

curl -X GET --insecure \
      -H "accept: application/octet-stream" \
      -H "Authorization: Bearer $webtoken" \
      --output thefile.bin \
      "https://localhost:16001/api/v1/file/e8c8ac1a-9e65-11eb-bcee-acde48001122"
