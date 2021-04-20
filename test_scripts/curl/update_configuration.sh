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

config=`./get_device.sh $1 | jq '.configuration'`
uuid=config=`./get_device.sh $1 | jq -r '.UUID'`

echo "======================="
echo "Original UUID: $uuid"
echo "======================="

echo $config

webtoken=`./login.sh | jq -r '.access_token'`

payload="{ \"serialNumber\" : \"$serial\" , \"UUID\" : 0, \"configuration\" : "$config" }"

curl -X POST "https://localhost:16001/api/v1/device/$serial/configure" \
      -H  "accept: application/json" \
      -H "Authorization: Bearer $webtoken" \
      --insecure -d "$payload"

newconfig=`./get_device.sh $1 | jq '.configuration'`
newuuid=config=`./get_device.sh $1 | jq -r '.UUID'`

echo "======================="
echo "New UUID: $newuuid"
echo "======================="
echo $newconfig
