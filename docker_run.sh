#!/bin/sh

HUBNAME=tip-tip-wlan-cloud-ucentral.jfrog.io
IMAGE_NAME=ucentralgw
DOCKER_NAME=$HUBNAME/$IMAGE_NAME:master

CONTAINER_NAME=ucentralgw

#stop previously running images
docker container stop $CONTAINER_NAME
docker container rm $CONTAINER_NAME --force

if [[ ! -d logs ]]
then
    mkdir logs
fi

if [[ ! -d uploads ]]
then
    mkdir uploads
fi

if [[ ! -d certs ]]
then
  echo "certs directory does not exist. Please create and add the proper certificates."
  exit 1
fi

if [[ ! -f ucentralgw.properties ]]
then
  echo "Configuration file ucentral.properties is missing in the current directory"
  exit 2
fi

docker run -d -p 15002:15002 \
              -p 16001:16001 \
              -p 16003:16003 \
              --init \
              --volume="$PWD:/ucentral-data" \
              -e UCENTRALGW_ROOT="/ucentral-data" \
              -e UCENTRALGW_CONFIG="/ucentral-data" \
              --name="ucentralgw" $DOCKER_NAME

