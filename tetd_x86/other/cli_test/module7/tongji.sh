#!/bin/bash

i=1
while [ $i -le 100 ]
do
    sh t2.sh
    i=$((i+1))
done
echo "$((i-1)) times finished."

