%{
    #include "syntax.tab.h"
    #include <stdlib.h>
    #include <string.h>
    #include "tree.h"
    #include "irgen.h"

    extern int err;
    int yycolumn = 1;
    #define YY_USER_ACTION { \
        yylloc.first_line = yylloc.last_line = yylineno;\
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng; \
    }

    void comment();

    #define extern_tnode(str) {\
        tnode *p = new_node(#str, yylineno); \
        p->syntax_label = _##str##_; \
        yylval.type_tnode_ptr = p; \
    }

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

BLANK   [\n\f\v\t\r ]+
NBLANK  [^\n\f\v\t\r ]
%%
\n  {yycolumn ++;}
"//"[^\n]*

"/*"    {
    comment();
}

{SEMI}  {
    extern_tnode(SEMI)
    return SEMI; printf("SEMI\n");}
{COMMA} {
    extern_tnode(COMMA)
    return COMMA; printf("COMMA\n");}
{ASSIGNOP}  {
    extern_tnode(ASSIGNOP)
    return ASSIGNOP; printf("ASSIGNOP\n");}
{RELOP} {
    tnode *p = new_node("RELOP", yylineno); 
    p->syntax_label = _RELOP_; 
    yylval.type_tnode_ptr = p; 
    if(yytext[0] == '=') p->rk = EQ;
    else if(yytext[0] == '!') p->rk = NE;
    else if(yytext[0] == '>') {
        if(yytext[1] == '=') p->rk = GE;
        else p->rk = GT;
    }
    else if(yytext[0] == '<') {
        if(yytext[1] == '=') p->rk = LE;
        else p->rk = LT;
    }
    return RELOP; printf("RELOP\n");}
{PLUS}  {
    extern_tnode(PLUS)
    return PLUS; printf("PLUS\n");}
{MINUS} {
    extern_tnode(MINUS)
    return MINUS; printf("MINUS\n");}
{STAR}  {
    extern_tnode(STAR)
    return STAR; printf("STAR\n");}
{DIV}   {
    extern_tnode(DIV)
    return DIV; printf("DIV\n");}
{AND}   {
    extern_tnode(AND)
    return AND; printf("AND\n");}
{OR}    {
    extern_tnode(OR)
    return OR; printf("OR\n");}
{DOT}   {
    extern_tnode(DOT)
    return DOT; printf("DOT\n");}
{NOT}   {
    extern_tnode(NOT)
    return NOT; printf("NOT\n");}
{LP}    {
    extern_tnode(LP)
    return LP; printf("LP\n");}
{RP}    {
    extern_tnode(RP)
    return RP; printf("RP\n");}
{LB}    {
    extern_tnode(LB)
    return LB; printf("LB\n");}
{RB}    {
    extern_tnode(RB)
    return RB; printf("RB\n");}
{LC}    {
    extern_tnode(LC)
    return LC; printf("LC\n");}
{RC}    {
    extern_tnode(RC)
    return RC; printf("RC\n");}
{STRUCT}    {
    extern_tnode(STRUCT)
    return STRUCT; printf("STRUCT\n");}
{RETURN}    {
    extern_tnode(RETURN)
    return RETURN; printf("RETURN\n");}
{IF}    {
    extern_tnode(IF)
    return IF; printf("IF\n");}
{ELSE}  {
    extern_tnode(ELSE)
    return ELSE; printf("ELSE\n");}
{WHILE} {
    extern_tnode(WHILE)
    return WHILE; printf("WHILE\n");}
{TYPE}  {
    tnode *p = new_node("TYPE", yylineno);
    yylval.type_tnode_ptr = p;
    if(strcmp(yytext, "int") == 0) p->intval = 0;
    else p->intval = 1;
    p->syntax_label = _TYPE_;
    return TYPE; printf("TYPE: %s\n", yytext);}
{FLOAT} {
    tnode *p = new_node("FLOAT", yylineno);
    p->floval = atof(yytext);
    yylval.type_tnode_ptr = p;
    p->syntax_label = _FLOAT_;
    return FLOAT; printf("FLOAT: %s\n", yytext);}
{INT}   {
    tnode *p = new_node("INT", yylineno);
    if(yytext[0] == '0') {
        if(yytext[1] == 'x' || yytext[1] == 'X') sscanf(yytext, "%x", &p->intval);
        else sscanf(yytext, "%o", &p->intval);
    } else sscanf(yytext, "%d", &p->intval);
    yylval.type_tnode_ptr = p;
    p->syntax_label = _INT_;
    return INT; printf("INT: %s\n", yytext);}
{ID}    {
    tnode *p = new_node("ID", yylineno);
    p->strval = malloc(sizeof(char) * (yyleng + 1));
    strcpy(p->strval, yytext);
    yylval.type_tnode_ptr = p;
    p->syntax_label = _ID_;
    return ID; printf("ID: %s\n", yytext);}

{BLANK} {}
{NBLANK} {fprintf(stderr, "Error type A at Line %d: Mysterious characters \"%s\"\n", yylineno, yytext); err = 1;}
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
