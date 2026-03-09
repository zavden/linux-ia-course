#!/bin/bash
mkdir -p proyecto/src proyecto/docs
touch proyecto/src/main.c proyecto/src/utils.c proyecto/src/math.c
touch proyecto/docs/readme.txt proyecto/docs/changelog.txt
mv proyecto/docs/*.txt proyecto/
