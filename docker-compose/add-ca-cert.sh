#!/usr/bin/env bash
set -e

SERVICES="ucentralgw.wlan.local ucentralsec.wlan.local"

for i in $SERVICES; do
    docker-compose exec -T $i apk add ca-certificates
    docker cp certs/restapi-ca.pem ucentral_$i\_1:/usr/local/share/ca-certificates/
    docker-compose exec -T $i update-ca-certificates
done
