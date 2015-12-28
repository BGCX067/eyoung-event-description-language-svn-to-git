#!/bin/sh

SRC="events"
FILE_DIR="file"
FRAG_DIR="frag"
events=`cat $SRC`

for event in $events
do
	file="$FILE_DIR/event-$event.file.true"
	frag="$FRAG_DIR/event-$event.frag.true"
	touch $file
	echo "<html>" >> $file
	echo "	<body $event=\"javascript:alert('abc')\">" >> $file
	echo "		aaa" >> $file
	echo "	</body>" >> $file
	echo "</html>" >> $file
	cp -rf $file $frag
done
