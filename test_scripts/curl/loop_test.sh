#!/bin/zsh
i=0

until [ $i -gt 1000 ]
do
  echo i: $i
  ./cli configure 24f5a207a130 ea8300.json
  ((i=i+1))
done
