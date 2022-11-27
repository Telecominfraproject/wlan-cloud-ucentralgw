#!/bin/bash

private_key_name="$1"-private-key.pem
public_key_name="$1"-public-key.pem

echo "Generating private key..."
openssl genrsa -out ${private_key_name} 2048
echo "Extracting public key"
openssl rsa -in ${private_key_name} -outform PEM -pubout -out ${public_key_name}