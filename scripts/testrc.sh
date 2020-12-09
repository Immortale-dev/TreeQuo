#!/bin/bash

count=1000

set -e

if [[ ! -z $1 ]]; then 
	count=$1
fi

make rc
i=$count
while [[ $i -gt 0 ]]; do
	echo "Test $i:"
	rm -rf ./tmp/t3
	./rc_test.exe
	((i--))
	if [[ $? -ne 0 ]]; then
		echo "ERROR!!!"
		exit 1
	fi
done
