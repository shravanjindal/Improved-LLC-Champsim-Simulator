#!/bin/bash

mkdir -p $PWD/../dpc3_traces
while read LINE
do
    wget -P $PWD/../dpc3_traces -c http://dpc3.compas.cs.stonybrook.edu/champsim-traces/speccpu/$LINE
done < dpc3_max_simpoint.txt
