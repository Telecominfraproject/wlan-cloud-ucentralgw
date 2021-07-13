#!/bin/bash

daemon=ucentralgw

if [[ "$1" == "aws" ]]
then
	cp ${daemon}.properties.aws ${daemon}.properties
	. ./set_env.sh
	cd cmake-build
	./${daemon} --daemon
	echo "Running AWS version as daemon..."
fi

if [[ "$1" == "priv" ]]
then
  cp ${daemon}.properties.priv ${daemon}.properties
  . ./set_env.sh
  cd cmake-build
  ./${daemon} --daemon
  echo "Running private version as daemon..."
fi



