%{
    #include "syntax.tab.h"
    #include <stdlib.h>
    #include <string.h>
    #include "tree.h"

    extern int err;
    int yycolumn = 1;
    #define YY_USER_ACTION { \
        yylloc.first_line = yylloc.last_line = yylineno;\
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng; \
    }

    void comment();

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
\n  {yycolumn ++;}
"//"[^\n]*

"/*"    {
    comment();
}

{SEMI}  {
    tnode *p = new_node("SEMI", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return SEMI; printf("SEMI\n");}
{COMMA} {
    tnode *p = new_node("COMMA", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return COMMA; printf("COMMA\n");}
{ASSIGNOP}  {
    tnode *p = new_node("ASSIGNOP", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return ASSIGNOP; printf("ASSIGNOP\n");}
{RELOP} {
    tnode *p = new_node("RELOP", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return RELOP; printf("RELOP\n");}
{PLUS}  {
    tnode *p = new_node("PLUS", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return PLUS; printf("PLUS\n");}
{MINUS} {
    tnode *p = new_node("MINUS", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return MINUS; printf("MINUS\n");}
{STAR}  {
    tnode *p = new_node("STAR", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return STAR; printf("STAR\n");}
{DIV}   {
    tnode *p = new_node("DIV", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return DIV; printf("DIV\n");}
{AND}   {
    tnode *p = new_node("AND", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return AND; printf("AND\n");}
{OR}    {
    tnode *p = new_node("OR", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return OR; printf("OR\n");}
{DOT}   {
    tnode *p = new_node("DOT", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return DOT; printf("DOT\n");}
{NOT}   {
    tnode *p = new_node("NOT", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return NOT; printf("NOT\n");}
{LP}    {
    tnode *p = new_node("LP", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return LP; printf("LP\n");}
{RP}    {
    tnode *p = new_node("RP", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return RP; printf("RP\n");}
{LB}    {
    tnode *p = new_node("LB", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return LB; printf("LB\n");}
{RB}    {
    tnode *p = new_node("RB", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return RB; printf("RB\n");}
{LC}    {
    tnode *p = new_node("LC", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return LC; printf("LC\n");}
{RC}    {
    tnode *p = new_node("RC", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return RC; printf("RC\n");}
{STRUCT}    {
    tnode *p = new_node("STRUCT", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return STRUCT; printf("STRUCT\n");}
{RETURN}    {
    tnode *p = new_node("RETURN", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return RETURN; printf("RETURN\n");}
{IF}    {
    tnode *p = new_node("IF", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return IF; printf("IF\n");}
{ELSE}  {
    tnode *p = new_node("ELSE", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return ELSE; printf("ELSE\n");}
{WHILE} {
    tnode *p = new_node("WHILE", TERMINALS_LINE);
    yylval.type_tnode_ptr = p;
    return WHILE; printf("WHILE\n");}
{TYPE}  {
    tnode *p = new_node("TYPE", -TYPE);
    if(strcmp(yytext, "int") == 0) p->intval = 0;
    else p->intval = 1;
    yylval.type_tnode_ptr = p;
    return TYPE; printf("TYPE: %s\n", yytext);}
{FLOAT} {
    tnode *p = new_node("FLOAT", -FLOAT);
    p->floval = atof(yytext);
    yylval.type_tnode_ptr = p;
    return FLOAT; printf("FLOAT: %s\n", yytext);}
{INT}   {
    tnode *p = new_node("INT", -INT);
    if(yytext[0] == '0') {
        if(yytext[1] == 'x' || yytext[1] == 'X') sscanf(yytext, "%x", &p->intval);
        else sscanf(yytext, "%o", &p->intval);
    } else sscanf(yytext, "%d", &p->intval);
    yylval.type_tnode_ptr = p;
    return INT; printf("INT: %s\n", yytext);}
{ID}    {
    tnode *p = new_node("ID", -ID);
    p->strval = malloc(sizeof(char) * (yyleng + 1));
    strcpy(p->strval, yytext);
    yylval.type_tnode_ptr = p;
    return ID; printf("ID: %s\n", yytext);}

{WHITE} {}
{NWHITE} {fprintf(stderr, "Error type A at Line %d: Mysterious characters \"%s\"\n", yylineno, yytext); err = 1;}
%%
void comment(void) {
    char c, prev = 0;

    while((c = input()) != 0) {
        if(c == '/' && prev == '*')
        return;
        prev = c;
    }
    error("unterminated comment");
}
