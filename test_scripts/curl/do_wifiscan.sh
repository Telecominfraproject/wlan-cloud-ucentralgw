#!/bin/bash

if [ $# -eq 0 ]; then
  if [ "$test_device" = "" ]; then
    echo "You must supply the serial number of device to test or set <test_device> to the serial number you want to test."
    exit 1
  else
    serial=$test_device
  fi
else
  serial=$1
fi

payload="{ \"serialNumber\" : \"$serial\" , \"verbose\" : true }"

webtoken=`./login.sh | jq -r '.access_token'`

curl -X POST \
      --max-time 20 \
      -H "accept: application/json" \
      -H "Authorization: Bearer $webtoken" \
      --insecure -d "$payload" \
      "https://ucentral.dpaas.arilia.com:16001/api/v1/device/$serial/wifiscan"
