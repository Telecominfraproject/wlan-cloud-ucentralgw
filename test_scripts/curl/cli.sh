#!/bin/bash

host="ucentral.dpaas.arilia.com:16001"
webtoken=""
rfile=result.json
username="support@example.com"
password="support"

login() {
  payload="{ \"userId\" : \"$username\" , \"password\" : \"$password\" }"
  webtoken=`curl -s -X POST -H "Content-Type: application/json" -d "$payload" "https://$host/api/v1/oauth2" | jq -r '.access_token'`
}

logout() {
  curl  -s -X DELETE -H "Content-Type: application/json" \
        -H "Authorization: Bearer $webtoken" \
        "https://$host/api/v1/oauth2/$webtoken"
}

getdevice() {
  curl  -s -X GET "https://$host/api/v1/device/$1" \
        -H "accept: application/json" \
        -H "Authorization: Bearer $webtoken" > $rfile
  cat $rfile | jq
}

getcommand() {
  curl  -s -X GET "https://$host/api/v1/command/$1" \
        -H "accept: application/json" \
        -H "Authorization: Bearer $webtoken" > $rfile
  cat $rfile | jq
}

getcapabilities() {
  curl  -s -X GET "https://$host/api/v1/device/$1/capabilities" \
        -H "accept: application/json" \
        -H "Authorization: Bearer $webtoken" > $rfile
  cat $rfile | jq
}

deletecapabilities() {
  curl  -s -X DELETE "https://$host/api/v1/device/$1/capabilities" \
        -H "accept: application/json" \
        -H "Authorization: Bearer $webtoken"
}

listdevices() {
  curl  -s -X GET "https://$host/api/v1/devices" \
        -H "accept: application/json" \
        -H "Authorization: Bearer $webtoken" > $rfile
  cat $rfile | jq
}

deletedevice() {
  curl  -s -X DELETE "https://$host/api/v1/device/$1" \
        -H "accept: application/json" \
        -H "Authorization: Bearer $webtoken"
}

createdevice() {
  payload="{ \"serialNumber\": \"$1\", \"UUID\": 1234456, \"configuration\" : `cat default_config.json` , \"deviceType\": \"AP_Default\",\"location\": \"\", \"macAddress\": \"24:f5:a2:07:a1:30\", \"manufacturer\": \"Linksys EA8300 (Dallas)\", \"notes\": \"auto created device.\", \"owner\": \"\" }"
  curl  -s -X POST "https://$host/api/v1/device/$1" \
        -H "accept: application/json" \
        -H "Authorization: Bearer $webtoken" \
        -d "$payload"  > $rfile
  cat $rfile | jq
}

reboot() {
  payload="{ \"serialNumber\" : \"$1\" }"
  curl  -s -X POST "https://$host/api/v1/device/$1/reboot" \
        -H "accept: application/json" \
        -H "Authorization: Bearer $webtoken" \
        -d "$payload"  > $rfile
  cat $rfile | jq
}

setloglevel() {
  # Possible log levels: none, fatal, critical, error, warning, notice, information, debug, trace
  # Possible subsystem: ufileuploader, websocket, storage, restapi, commandmanager, auth, deviceregistry, all
  payload='{ "command" : "setloglevel" , "parameters" : [ { "name" : "websocket" , "value" : "information" } ] }'
  curl -X POST "https://$host/api/v1/system" \
        -H  "accept: application/json" \
        -H "Authorization: Bearer $webtoken" \
        -d "$payload"
}

login

if [[ $1 == "getdevice" ]]
then
  getdevice $2
elif [[ $1 == "getcommand" ]]
then
  getcommand $2
elif [[ $1 == "getcapabilities" ]]
then
  getcapabilities $2
elif [[ $1 == "deletecapabilities" ]]
then
  deletecapabilities $2
elif [[ $1 == "listdevices" ]]
then
  listdevices
elif [[ $1 == "deletedevice" ]]
then
  deletedevice $2
elif [[ $1 == "createdevice" ]]
then
  createdevice $2 $3
elif [[ $1 == "reboot" ]]
then
  reboot $2
elif [[ $1 == "setloglevel" ]]
then
  setloglevel
fi

logout
