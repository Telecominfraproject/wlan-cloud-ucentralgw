#!/usr/bin/env bash

SERVICES="ucentral_ucentralgw.wlan.local_1 ucentral_ucentralsec.wlan.local_1"
export RESTAPI_CA_CERT=$(cat certs/restapi-ca.pem)

for i in $SERVICES; do
    docker-compose exec $i apk add ca-certificates
    docker-compose exec $i echo "$RESTAPI_CA_CERT" > /usr/local/share/ca-certificates/restapi-cert.pem
    docker-compose exec $i update-ca-certificates 
done
