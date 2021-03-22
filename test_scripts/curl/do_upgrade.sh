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

payload="{ \"serialNumber\" : \"$serial\" , \"uri\" : \"https://somelocation.com/file=newversion.bin\", \"digest\" : \"1234567890\" }"

echo $payload

webtoken=`./login.sh | jq -r '.access_token'`

curl -X POST \
      -H "accept: application/json" \
      -H "Authorization: Bearer $webtoken" \
      --insecure -d "$payload" \
      "https://localhost:16001/api/v1/device/$serial/upgrade"

