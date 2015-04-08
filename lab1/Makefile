CC=gcc
CFLAGS=-lfl -O2 -o
RES=$(shell find -name "scanner" -o -name "lex.yy.c" -o -name "parser")

parser: lex.yy.c syntax.tab.c main.c
	$(CC) main.c syntax.tab.c -ly $(CFLAGS) parser
scanner: lex.yy.c
	$(CC) lex.yy.c $(CFLAGS) scanner
	
lex.yy.c: lexical.l
	flex lexical.l

clean:
	rm $(RES)
