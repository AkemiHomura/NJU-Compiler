#ifndef __TREE_H__
#define __TREE_H__

enum syntax_label_group {
    _INT_ = 1, _TYPE_ = 2, _FLOAT_ = 3, _ID_ = 4, _SEMI_ = 5,
    _COMMA_ = 6, _ASSIGNOP_ = 7, _RELOP_ = 8, _PLUS_ = 9,
    _MINUS_ = 10, _STAR_ = 11, _DIV_ = 12, _AND_ = 13, _OR_ = 14,
    _NOT_ = 15, _LP_ = 16, _RP_ = 17, _LB_ = 18, _RB_ = 19,
    _LC_ = 20, _RC_ = 21, _STRUCT_ = 22, _RETURN_ = 23, _IF_ = 24,
    _ELSE_ = 25, _WHILE_ = 26, _DOT_ = 27,

    _Program_ = 27, _ExtDefList_ = 28, _ExtDef_ = 29,
    _Specifier_ = 30, _ExtDecList_ = 31, _FunDec_ = 32,
    _CompSt_ = 33, _VarDec_ = 34, _StructSpecifier_ = 35,
    _OptTag_ = 36, _Tag_ = 37, _VarList_ = 38, _ParamDec_ = 39,
    _StmtList_ = 40, _Stmt_ = 41, _Exp_ = 42, _DefList_ = 43,
    _DecList_ = 44, _Dec_ = 45, _Def_ = 46, _Args_ = 47
};
typedef enum syntax_label_group syntax_label_group;

struct tnode {
    char *info;
    int syntax_label;
    int line;
    union {
        int intval;
        double floval;
        char *strval;
    };
    struct tnode *parent;
    struct tnode *son[7];
    int snum;
};

typedef struct tnode tnode;

#define TERMINALS_LINE -1

tnode* new_node(char *, int);
void link_node(tnode *, tnode *);

#endif
