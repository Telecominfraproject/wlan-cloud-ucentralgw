#!/usr/bin/env bash

echo "This test script "

echo "Getting all serial numbers.."
./cli listdevices
cp result.json devices.json
jq -r '.devices[].serialNumber' devices.json > serialNumbers
while IFS= read -r serialNumber
do
  echo "Serial Number: $serialNumber"
  ./cli rtty "$serialNumber" noconnect
done < serialNumbers

