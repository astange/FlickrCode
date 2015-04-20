#!/bin/bash

destdir=/tmp/test/master.node

for i in `seq 1 100`;
do
    echo "post i.jpg" >> "$destdir"
done

