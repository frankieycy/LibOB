#!/bin/bash
if [ -z "$1" ]; then
	file="main"
else
	file=$1
fi
g++ -std=c++17 -I lib lib/*.cpp run/${file}.cpp -o exe/${file}
if [ $? -eq 0 ]; then
	./exe/${file} 2>&1 | tee run.log
fi
