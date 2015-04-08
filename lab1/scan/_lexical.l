%{
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

{SEMI}  { printf("SEMI\n");}
{COMMA} { printf("COMMA\n");}
{ASSIGNOP}  { printf("ASSIGNOP\n");}
{RELOP} { printf("RELOP\n");}
{PLUS}  { printf("PLUS\n");}
{MINUS} { printf("MINUS\n");}
{STAR}  { printf("STAR\n");}
{DIV}   { printf("DIV\n");}
{AND}   { printf("AND\n");}
{OR}    { printf("OR\n");}
{DOT}   { printf("DOT\n");}
{NOT}   { printf("NOT\n");}
{LP}    { printf("LP\n");}
{RP}    { printf("RP\n");}
{LB}    { printf("LB\n");}
{RB}    { printf("RB\n");}
{LC}    { printf("LC\n");}
{RC}    { printf("RC\n");}
{STRUCT}    { printf("STRUCT\n");}
{RETURN}    { printf("RETURN\n");}
{IF}    { printf("IF\n");}
{ELSE}  { printf("ELSE\n");}
{WHILE} { printf("WHILE\n");}
{TYPE}  { printf("TYPE: %s\n", yytext);}
{FLOAT}  { printf("FLOAT: %s\n", yytext);}
{INT}   { printf("INT: %s\n", yytext);}
{ID}    { printf("ID: %s\n", yytext);}

{WHITE} {}
{NWHITE} {printf("Error type A at Line %d: Mysterious characters \"%s\"\n", yylineno, yytext);}
%%
int main(int argc, char** argv) {
    if (argc > 1) {
        if(!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }
    yylex();
    return 0;
}
