#!/usr/bin/env bash

start=11
finish=50
cert_life=365

for i in `eval echo {$start..$finish}`
do
  openssl x509 -signkey server-key.pem -in server.csr -req -days $cert_life -out dev-$i-cert.pem
done