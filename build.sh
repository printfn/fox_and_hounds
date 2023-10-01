#!/bin/sh
${CC:-gcc} fh.c -O0 -g -fanalyzer -Wall -Wextra -Wpedantic \
    -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes \
    -Wmissing-declarations -Wdeclaration-after-statement \
    -std=c89 -o fh
