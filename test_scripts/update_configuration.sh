#!/bin/zsh

serial=$1

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
