#!/bin/bash

if [ $# -eq 2 ]; then
  username=$1
  password=$2
else
  username="support@example.com"
  password="support"
fi

payload="{ \"userId\" : \"$username\" , \"password\" : \"$password\" }"

curl -v -X POST -H "Content-Type: application/json" -d "$payload" "https://ucentral.dpaas.arilia.com:16001/api/v1/oauth2"
