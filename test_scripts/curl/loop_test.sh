#!/bin/zsh
i=0

until [ $i -gt 1000 ]
do
  echo i: $i
  ./cli eventqueue 24f5a207a130
  ((i=i+1))
done
