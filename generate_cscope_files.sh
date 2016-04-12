#!/bin/bash
rm *.out
find . -name "*.h" -o -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.m" > cscope.files
cscope -bqk
