#!/usr/bin/env sh
for i in `find -type f -name "*.wav"`; do ./hash.sh $i; done
for i in `find -type f -name "*.md5"`; do cat $i; done
