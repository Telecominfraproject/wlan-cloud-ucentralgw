#!/usr/bin/env bash

pidfilename="${UCENTRALGW_ROOT}/data/pidfile"

if [[ -f "${pidfilename}" ]]
then
  kill -9 $(cat ${pidfilename})
fi
