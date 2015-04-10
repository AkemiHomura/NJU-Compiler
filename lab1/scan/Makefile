scanner: lexical.l
	flex lexical.l
	gcc lex.yy.c -lfl -O2 -o scanner

clean: 
	rm scanner lex.yy.c

