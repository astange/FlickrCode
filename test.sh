#!/bin/bash

destdir=/tmp/test/master.node

for i in `seq 1 100`;
do
    echo "get $i.jpg" >> "$destdir"
done

