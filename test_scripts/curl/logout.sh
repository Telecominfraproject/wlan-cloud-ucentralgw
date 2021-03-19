#!/bin/bash

webtoken=$1

curl -X DELETE https://localhost:16001/api/v1/oauth2/$webtoken \
      -H "Content-Type: application/json" \
      -H "Authorization: Bearer $webtoken" \
      --insecure
