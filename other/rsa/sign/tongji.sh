#!/bin/bash

i=1
while [ $i -le 100 ]
do
    ./a
    i=$((i+1))
done
echo "$((i-1)) times finished."

