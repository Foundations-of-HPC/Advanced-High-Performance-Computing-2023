#!/bin/bash

for i in $(seq $1)
do
	echo "------- i = $i -------"
	$2
done
