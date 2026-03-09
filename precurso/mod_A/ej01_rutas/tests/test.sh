#!/bin/bash
set -e
echo "Test 1: Check proyecto/src"
[ -d "proyecto/src" ]
[ -f "proyecto/src/main.c" ]
[ -f "proyecto/src/utils.c" ]
[ -f "proyecto/src/math.c" ]
echo "Test 2: Check proyecto/docs"
[ -d "proyecto/docs" ]
[ ! -f "proyecto/docs/readme.txt" ]
echo "Test 3: Check root txts"
[ -f "proyecto/readme.txt" ]
[ -f "proyecto/changelog.txt" ]
echo "PASSED"
