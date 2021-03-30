#!/bin/bash

UUID="abcd-12345-defg-1234567890"

curl \
  -F "name=$UUID" \
  -F "data=@test.bin" \
  https://localhost:16003/v1/upload/$UUID \
  --insecure

