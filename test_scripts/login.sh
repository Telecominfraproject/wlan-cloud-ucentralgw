#!/bin/bash

payload="{ \"userId\" : \"support@example.com\" , \"password\" : \"support\" }"

curl -X POST -H "Content-Type: application/json" -d "$payload" "https://localhost:16001/api/v1/oauth2" --insecure
