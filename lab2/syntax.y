%{
    #include <stdio.h>
    #include "tree.h"
    tnode *root = NULL;
    int err = 0;
    #include "lex.yy.c"

    #define YYERROR_VERBOSE 1
    #define yyerror(msg) _yyerror(msg)
    void _yyerror(const char *msg) {
    printf("Error type B at Line %d: %s\n", yylineno, msg);

    #define link(n) {\
        int i = n - 1; \
        for(; i >= 0; i --) link_node((yyval.type_tnode_ptr), (yyvsp[-i].type_tnode_ptr)); \
    }
}
%}
%union {
    struct tnode* type_tnode_ptr;
}

%token <type_tnode_ptr> INT TYPE
%token <type_tnode_ptr> FLOAT
%token <type_tnode_ptr> ID
%token <type_tnode_ptr> SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR NOT DOT LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE

%right ASSIGNOP
%left OR
%left AND 
%left RELOP
%left PLUS MINUS
%left STAR DIV 
%right NEG NOT 
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


/* non-terminals */
%type <type_tnode_ptr> Program ExtDefList ExtDef Specifier ExtDecList FunDec CompSt VarDec StructSpecifier OptTag Tag VarList ParamDec StmtList Stmt Exp DefList DecList Dec Def Args

%%
Program : ExtDefList {
            $$ = root = new_node("Program", @$.first_line);
            $$->syntax_label = _Program_;
            link_node($$, $1);
        }
        ;
ExtDefList : ExtDef ExtDefList {
            $$ = new_node("ExtDefList", @$.first_line);
            $$->syntax_label = _ExtDefList_;
            link(2);
           }
           | {
            $$ = NULL;
           }
           ;
ExtDef : Specifier ExtDecList SEMI {
            $$ = new_node("ExtDef", @$.first_line);
            $$->syntax_label = _ExtDef_;
        link(3);
       }
       | Specifier SEMI {
            $$ = new_node("ExtDef", @$.first_line);
            $$->syntax_label = _ExtDef_;
         link(2);
       }
       | Specifier FunDec CompSt {
            $$ = new_node("ExtDef", @$.first_line);
            $$->syntax_label = _ExtDef_;
        link(3);
       }
       ;
ExtDecList : VarDec {
            $$ = new_node("ExtDecList", @$.first_line);
            $$->syntax_label = _ExtDecList_;
            link_node($$, $1);
           }
           | VarDec COMMA ExtDecList {
            $$ = new_node("ExtDecList", @$.first_line);
            $$->syntax_label = _ExtDecList_;
            link(3);
           }
           ;

Specifier : TYPE {
            $$ = new_node("Specifier", @$.first_line);
            $$->syntax_label = _Specifier_;
            link_node($$, $1);
          }
          | StructSpecifier {
            $$ = new_node("Specifier", @$.first_line);
            $$->syntax_label = _Specifier_;
            link_node($$, $1);
          }
          ;
StructSpecifier : STRUCT OptTag LC DefList RC {
                $$ = new_node("StructSpecifier", @$.first_line);
                $$->syntax_label = _StructSpecifier_;
                link(5);
                }
                | STRUCT Tag {
                $$ = new_node("StructSpecifier", @$.first_line);
                $$->syntax_label = _StructSpecifier_;
                link_node($$, $1);
                link_node($$, $2);
                }
                ;
OptTag : ID {
        $$ = new_node("OptTag", @$.first_line);
        $$->syntax_label = _OptTag_;
        link_node($$, $1);
       }
       | {
        $$ = NULL;
       }
       ;
Tag : ID {
        $$ = new_node("Tag", @$.first_line);
        $$->syntax_label = _Tag_;
        link_node($$, $1);
    }
    ;

VarDec : ID {
        $$ = new_node("VarDec", @$.first_line);
        $$->syntax_label = _VarDec_;
        link_node($$, $1);
       }
       | VarDec LB INT RB {
        $$ = new_node("VarDec", @$.first_line);
        $$->syntax_label = _VarDec_;
        link(4);
       }
       ;
FunDec : ID LP VarList RP {
        $$ = new_node("FunDec", @$.first_line);
        $$->syntax_label = _FunDec_;
        link(3);
       }
       | ID LP RP {
        $$ = new_node("FunDec", @$.first_line);
        $$->syntax_label = _FunDec_;
        link(3);
       }
       ;
VarList : ParamDec COMMA VarList {
        $$ = new_node("VarList", @$.first_line);
        $$->syntax_label = _VarList_;
        link(3);
        }
        | ParamDec {
        $$ = new_node("VarList", @$.first_line);
        $$->syntax_label = _VarList_;
        link_node($$, $1);
        }
        ;
ParamDec : Specifier VarDec {
         $$ = new_node("ParamDec", @$.first_line);
         $$->syntax_label = _ParamDec_;
         link_node($$, $1);
         link_node($$, $2);
         }
         ;

CompSt : LC DefList StmtList RC {
        $$ = new_node("CompSt", @$.first_line);
        $$->syntax_label = _CompSt_;
        link(4);
       }
       ;
StmtList : Stmt StmtList {
        $$ = new_node("StmtList", @$.first_line);
        $$->syntax_label = _StmtList_;
        link_node($$, $1);
        link_node($$, $2);
         }
         | {
        $$ = NULL;
         }
         ;
Stmt : Exp SEMI {
        $$ = new_node("Stmt", @$.first_line);
        $$->syntax_label = _Stmt_;
        link_node($$, $1);
        link_node($$, $2);
     }
     | CompSt {
        $$ = new_node("Stmt", @$.first_line);
        $$->syntax_label = _Stmt_;
        link_node($$, $1);
     }
     | RETURN Exp SEMI {
        $$ = new_node("Stmt", @$.first_line);
        $$->syntax_label = _Stmt_;
        link(3);
     }
     | IF LP Exp RP Stmt    %prec LOWER_THAN_ELSE {
        $$ = new_node("Stmt", @$.first_line);
        $$->syntax_label = _Stmt_;
        link(5);
     }
     | IF LP Exp RP Stmt ELSE Stmt {
        $$ = new_node("Stmt", @$.first_line);
        $$->syntax_label = _Stmt_;
        link(7);
     }
     | WHILE LP Exp RP Stmt {
        $$ = new_node("Stmt", @$.first_line);
        $$->syntax_label = _Stmt_;
        link(5);
     }
     ;

DefList : Def DefList {
            $$ = new_node("DefList", @$.first_line);
            $$->syntax_label = _DefList_;
            link_node($$, $1);
            link_node($$, $2);
        }
        | {
            $$ = NULL;
        }
        ;
Def : Specifier DecList SEMI {
        $$ = new_node("Def", @$.first_line);
        $$->syntax_label = _Def_;
        link(3);
    }
    ;
DecList : Dec {
            $$ = new_node("DecList", @$.first_line);
            $$->syntax_label = _DecList_;
            link_node($$, $1);
        }
        | Dec COMMA DecList {
            $$ = new_node("DecList", @$.first_line);
            $$->syntax_label = _DecList_;
            link(3);
        }
        ;
Dec : VarDec {
        $$ = new_node("Dec", @$.first_line);
        $$->syntax_label = _Dec_;
        link_node($$, $1);
    }
    | VarDec ASSIGNOP Exp {
        $$ = new_node("Dec", @$.first_line);
        $$->syntax_label = _Dec_;
        link(3);
    }
    ;

Exp : Exp ASSIGNOP Exp {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
    }
    | Exp AND Exp {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
    }
    | Exp OR Exp {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
    }
    | Exp RELOP Exp {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
    }
    | Exp PLUS Exp {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
    }
    | Exp MINUS Exp {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
    }
    | Exp STAR Exp {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
    }
    | Exp DIV Exp {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
    }
    | LP Exp RP {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
    }
    | MINUS Exp %prec NEG {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link_node($$, $1);
        link_node($$, $2);
    }
    | NOT Exp {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link_node($$, $1);
        link_node($$, $2);
    }
    | ID LP Args RP {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(4);
    }
    | ID LP RP {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
    }
    | Exp LB Exp RB {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(4);
    }
    | Exp DOT ID {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
    }
    | ID {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link_node($$, $1);
    }
    | INT {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link_node($$, $1);
    }
    | FLOAT {
        $$ = new_node("Exp", @$.first_line);
        $$->syntax_label = _Exp_;
        link_node($$, $1);
    }
    ;
Args : Exp COMMA Args {
        $$ = new_node("Args", @$.first_line);
        $$->syntax_label = _Exp_;
        link(3);
     }
     | Exp {
        $$ = new_node("Args", @$.first_line);
        $$->syntax_label = _Exp_;
        link_node($$, $1);
     }
     ;

ExtDef: error SEMI {
                err = 1;
      }
      ;

Def : error SEMI {
                err = 1;
    }
    ;

Stmt : error SEMI {
                err = 1;
     }
     ;

CompSt : error RC {
                err = 1;
       }
       ;

Exp : error Exp {
                err = 1;
    }
    ;

%%


