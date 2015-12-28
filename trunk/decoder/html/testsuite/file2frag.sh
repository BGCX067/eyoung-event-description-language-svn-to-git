#!/bin/sh

FILE_DIR="file"
FRAG_DIR="frag"
files=`ls $FILE_DIR`

for file in $files
do
	target=`ls $FILE_DIR/$file | sed s/file/frag/g`
	cp $FILE_DIR/$file $target
done
