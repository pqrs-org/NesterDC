#!/bin/sh

#========================================
# make lineartable.h
gcc -o make_lineartable make_lineartable.c -lm
make_lineartable > tbl/lineartable.h
rm make_lineartable


#========================================
# make logtable.h
gcc -o make_logtable make_logtable.c -lm
make_logtable > tbl/logtable.h
rm make_logtable


