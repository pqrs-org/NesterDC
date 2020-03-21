#!/bin/sh

if [ $# -eq 0 ]; then
    echo "Usage: $(basename $0) kos-directory"
    exit 1;
fi

find $1 -type f | grep -v '/CVS' > kos-tree-type-f
find $1 -type d | grep -v '/CVS' > kos-tree-type-d

for f in $(cat kos-tree-type-f)
do
  echo "rm $f"
  echo "cvs remove $f"
done

cvs ci -m '' $1
cvs update $1
