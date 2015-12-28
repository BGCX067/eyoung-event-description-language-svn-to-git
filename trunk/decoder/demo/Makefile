all:
	bison -d -v -t -o test_parser.c test.y
	flex -P calc --noline --bison-bridge --header-file=test_lex.h -o test_lex.c test.l
	gcc -Wall -o test -g test_lex.c test_parser.c

clean:
	rm -rf test_parser.* test_lex.* test
