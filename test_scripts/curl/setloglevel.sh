#!/bin/bash

webtoken=`./login.sh | jq -r '.access_token'`

payload='{ "command" : "setloglevel" , "parameters" : [ { "name" : "websocket" , "value" : "information" } ] }'

# Possible log levels: none, fatal, critical, error, warning, notice, information, debug, trace
# Possible subsystem: ufileuploader, websocket, storage, restapi, commandmanager, auth, deviceregistry, all

curl -X POST "https://localhost:16001/api/v1/system" \
      -H  "accept: application/json" \
      -H "Authorization: Bearer $webtoken" \
      --insecure -d "$payload"
