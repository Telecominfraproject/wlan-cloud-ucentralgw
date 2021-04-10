#!/bin/bash

hn=$(hostname)
howmany=10
cert_life=365
subject="/C=CA/ST=British Columbia/L=Vancouver/O=Arilia Wireless/OU=Engineering/CN=$hn/emailAddress=support@example.com"

openssl genrsa -out server-key.pem 2048
openssl req -new -key server-key.pem -subj "$subject" -out server.csr
openssl x509 -req -days $cert_life -in server.csr -signkey server-key.pem -out server-cert.pem

for i in `eval echo {1..$howmany}`
do
  openssl x509 -signkey server-key.pem -in server.csr -req -days $cert_life -out dev-$i-cert.pem
done
