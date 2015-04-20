#!/bin/bash

destdir=/tmp/test.txt

for i in `seq 1 10`;
do
    echo "$i.jpg" >> "$destdir"
done

