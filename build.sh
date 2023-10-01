#!/bin/sh
${CC:-gcc} fh.c -O0 -g -fanalyzer -Wall -Wextra -Wpedantic -std=c89 -o fh
