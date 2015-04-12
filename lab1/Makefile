CC=gcc
CFLAGS=-lfl -O2 -o
RES=$(shell find -name "lex.yy.c" -o -name "parser" -o -name "syntax.tab.*")

parser: lex.yy.c syntax.tab.c main.c
	$(CC) main.c syntax.tab.c -ly $(CFLAGS) parser

debug: lex.yy.c syntax.tab.c main.c
	$(CC) main.c syntax.tab.c -ly $(CFLAGS) parser -D YYDEBUG

syntax.tab.c syntax.tab.h: syntax.y
	bison -d syntax.y

lex.yy.c: lexical.l syntax.tab.h
	flex lexical.l

syntax-debug: syntax.y
	bison -d -v syntax.y

clean:
	rm $(RES) $(shell find -name "syntax.output")
