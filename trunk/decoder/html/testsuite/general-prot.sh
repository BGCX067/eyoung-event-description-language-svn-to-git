#!/bin/sh

SRC="general"
FILE_DIR="file"
FRAG_DIR="frag"
prots=`cat $SRC`

for prot in $prots
do
	file="$FILE_DIR/prot-$prot.file.true"
	frag="$FRAG_DIR/prot-$prot.frag.true"
	touch $file
	echo "<html>" >> $file
	echo "	<body>" >> $file
	echo "		<b $prot=\"javascript:alert('abc')\">aaa</b>" >> $file
	echo "	</body>" >> $file
	echo "</html>" >> $file
	cp -rf $file $frag
done
