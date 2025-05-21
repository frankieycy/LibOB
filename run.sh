#!/bin/bash
if [ -z "$1" ]; then
	file="main"
else
	file=$1
fi
g++ -Ilib -std=c++17 -Wall -Wextra -O2 $(find run lib -name '*.cpp') -o exe/${file}
if [ $? -eq 0 ]; then
	./exe/${file} 2>&1 | tee run.log
fi
