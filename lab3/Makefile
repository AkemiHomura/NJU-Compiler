CC=gcc
CFLAGS=-Werror -lfl -g -O2 -o
LEXC_AND_YACCC=$(shell find -name "lex.yy.c" -o -name "syntax.tab.*")
YACC_DEBUG_OUT=$(shell find -name "syntax.output")
RES=$(shell find -name "parser")
INCLUDE=$(shell find -name "*.h")

parser: lex.yy.c syntax.tab.c main.c tree.c ta.c $(INCLUDE)
	$(CC) main.c syntax.tab.c tree.c ta.c -ly $(CFLAGS) parser

debug: lex.yy.c syntax.tab.c main.c
	$(CC) main.c syntax.tab.c -ly $(CFLAGS) parser -D YYDEBUG

syntax.tab.c syntax.tab.h: syntax.y
	bison -d syntax.y

lex.yy.c: lexical.l syntax.tab.h
	flex lexical.l

syntax-debug: syntax.y
	bison -d -v syntax.y

single_file_test: parser
	./parser test.cmm

test: parser
	for t in $(shell find test/*); \
		do 	echo "$$t" ; \
		cat "$$t"; \
		./parser "$$t" ; \
		echo "\n" ; \
		done

.PHONY: clean clean-respreserved 
clean: clean-respreserved
	rm $(RES) 

clean-respreserved:
	rm $(YACC_DEBUG_OUT) $(LEXC_AND_YACCC)
