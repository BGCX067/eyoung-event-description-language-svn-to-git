#!/bin/sh

TEST_DIR="testsuite"
LOG_FILE="html.log"
EXE_FILE="./html"

if [ -e $LOG_FILE ] ; then
	rm -rf $LOG_FILE
fi
touch $LOG_FILE

get_type()
{
	case "$1" in
	"xss")
		return 1
	;;
	"file")
		return 2
	;;
	"frag")
		return 3
	;;
	*)
		return 0
	;;
	esac
}

get_result()
{
	case "$1" in
	"true")
		return 1
	;;
	"false")
		return 0
	;;
	*)
		return 2
	;;
	esac
}

FILES=`ls $TEST_DIR`
for FILE in $FILES
do
	PART2=`echo $FILE | awk -F . {'print $2'}`
	PART3=`echo $FILE | awk -F . {'print $3'}`

	get_type $PART2
	TYPE=$?

	get_result $PART3
	RESULT=$?
	
	$EXE_FILE -f $TEST_DIR/$FILE -t $TYPE -r $RESULT >> $LOG_FILE 2>&1
done
