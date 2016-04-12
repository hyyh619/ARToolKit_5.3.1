#!/bin/bash
rm tags
find . -name "*.h" -o -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.m" > ctags.files
/usr/local/Cellar/ctags/5.8_1/bin/ctags -L ctags.files
