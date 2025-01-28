#!/bin/bash

for est in $(seq 1 3); do
	for prob in $(seq 0 10 100); do
		python3 main.py -eA $est -eB $est -pA $prob;
	done
done
