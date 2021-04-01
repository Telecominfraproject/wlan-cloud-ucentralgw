#!/bin/sh

# Removes all local images. This is dangerous but good when debugging
docker rmi -f $(docker images -a -q)

