#!/bin/sh

for f in *.h 
do
  ruby -ne 'if /friend void adopt_MPRD/ =~ $_ then print "  friend class NES_SNSS;\n" else print $_ end' $f | \
      ruby -ne 'print $_ unless /friend int/ =~ $_' | \
      ruby -ne 'print $_ unless /friend void/ =~ $_' > HOGE
  mv HOGE $f
done
