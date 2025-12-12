#!/bin/bash
if [ -z "$1" ]; then
	file="main"
else
	file=$1
fi
g++ -ILib -std=c++17 -Wall -Wextra -O2 $(find Run Lib -name '*.cpp') -o Exe/${file}
if [ $? -eq 0 ]; then
	./Exe/${file} 2>&1 | tee run.log
fi
