CC=gcc
CFLAGS=-Werror -lfl -O2 -o
LEXC_AND_YACCC=$(shell find -name "lex.yy.c" -o -name "syntax.tab.*")
YACC_DEBUG_OUT=$(shell find -name "syntax.output")
RES=$(shell find -name "parser")

parser: lex.yy.c syntax.tab.c main.c tree.c ta.c
	$(CC) main.c syntax.tab.c tree.c ta.c -ly $(CFLAGS) parser

debug: lex.yy.c syntax.tab.c main.c
	$(CC) main.c syntax.tab.c -ly $(CFLAGS) parser -D YYDEBUG

syntax.tab.c syntax.tab.h: syntax.y
	bison -d syntax.y

lex.yy.c: lexical.l syntax.tab.h
	flex lexical.l

syntax-debug: syntax.y
	bison -d -v syntax.y

.PHONY: clean test clean-respreserved
test:
	touch test.out
	rm test.out
	touch test.out
	for t in $(shell find test/*); \
		do 	echo "$$t" >> test.out; \
			./parser "$$t" >> test.out; \
			echo "\n" >> test.out; \
		done

clean: clean-respreserved
	rm $(RES) 

clean-respreserved:
	rm $(YACC_DEBUG_OUT) $(LEXC_AND_YACCC)
