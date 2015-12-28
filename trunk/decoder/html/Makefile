all:
	bison -d -v -t -o html_parser.c html.y
	flex html.l
	gcc -DHTML_DEBUG=1 -Wall -Wno-sign-compare -D_GNU_SOURCE -o html -g html_lex.c html_parser.c html_mem.c html_util.c main.c

clean:
	rm -rf html_parser.* html_lex.* html html.log

test: all
	./test.sh
