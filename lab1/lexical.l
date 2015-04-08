%{
    #include "syntax.tab.h"
%}
%option yylineno

SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS \-
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT !
TYPE int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while

INT     (0x|0X)[0-9a-fA-F]{1,8}|0[0-3]?[0-7]{1,10}|4[0-2][0-9]{8}|[1-3][0-9]{9}|[1-9][0-9]{0,8}|0
FLOAT   [0-9]+\.[0-9]+|[0-9]*\.[0-9]+[Ee][+-][0-9]+
ID      [_a-zA-Z][_a-zA-Z0-9]*

WHITE   [\n\f\v\t\r ]+
NWHITE  [^\n\f\v\t\r ]
%%
"//"    {
    char c = input();
    while(c != '\n') c = input();
}
"/*"    {
    char c = input();
    while(1) {
        if(c == '*') {
            c = input();
            if(c == '/') break;
        }
    }
}

{SEMI}  {return SEMI; printf("SEMI\n");}
{COMMA} {return COMMA; printf("COMMA\n");}
{ASSIGNOP}  {return ASSIGNOP; printf("ASSIGNOP\n");}
{RELOP} {return RELOP; printf("RELOP\n");}
{PLUS}  {return PLUS; printf("PLUS\n");}
{MINUS} {return MINUS; printf("MINUS\n");}
{STAR}  {return STAR; printf("STAR\n");}
{DIV}   {return DIV; printf("DIV\n");}
{AND}   {return AND; printf("AND\n");}
{OR}    {return OR; printf("OR\n");}
{DOT}   {return DOT; printf("DOT\n");}
{NOT}   {return NOT; printf("NOT\n");}
{LP}    {return LP; printf("LP\n");}
{RP}    {return RP; printf("RP\n");}
{LB}    {return LB; printf("LB\n");}
{RB}    {return RB; printf("RB\n");}
{LC}    {return LC; printf("LC\n");}
{RC}    {return RC; printf("RC\n");}
{STRUCT}    {return STRUCT; printf("STRUCT\n");}
{RETURN}    {return RETURN; printf("RETURN\n");}
{IF}    {return IF; printf("IF\n");}
{ELSE}  {return ELSE; printf("ELSE\n");}
{WHILE} {return WHILE; printf("WHILE\n");}
{TYPE}  {return TYPE; printf("TYPE: %s\n", yytext);}
{FLOAT}  {return FLOAT; printf("FLOAT: %s\n", yytext);}
{INT}   {return INT; printf("INT: %s\n", yytext);}
{ID}    {return ID; printf("ID: %s\n", yytext);}

{WHITE} {}
{NWHITE} {printf("Error type A at Line %d: Mysterious characters \"%s\"\n", yylineno, yytext);}
%%
