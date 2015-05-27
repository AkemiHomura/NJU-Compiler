#ifndef __IRGEN_H__
#define __IRGEN_H__
#include "list.h"
#include "tree.h"
#include <stdio.h>

#define new(type, ...) (type *)({ \
    type *__ = (type *)malloc(sizeof(type)); \
    *__ = (type){__VA_ARGS__}; \
    __; \
})
extern list_head code;

typedef enum {
    IR_ASSIGN, IR_ADD, IR_SUB, IR_MUL, IR_DIV, IR_RETURN,
    IR_LABEL, IR_GOTO, IR_IFGOTO, IR_READ, IR_WRITE, IR_CALL,
    IR_ARG, IR_FUNCTION, IR_PARAM, IR_DEC, IR_RIGHTAT
} ir_kind;

typedef enum {
    VARIABLE, CONSTANT, TEMPVAR, VADDRESS, LABEL,
    FUNCTION, TADDRESS
} op_kind;

struct Label {
    int no;
    struct Label *next;
};
typedef struct Label Label;

struct Operand {
    op_kind kind;
    union {
        int var_no; // tempvar, variable
        Label* label; // label
        char* value; // function
        int cons; // constant
        struct Operand* name; //vaddress, taddress
    } u;
    struct Operand* next;
};
typedef struct Operand Operand;

struct InterCode {
    ir_kind kind;
    union {
        /* return, label, goto, read, write, arg, function, param */
        struct { Operand *op; } one;
        /* assign, call */
        struct { Operand *left, *right; } assign;
        /* add, sub, mul, div */
        struct { Operand *result, *op1, *op2; } binop;
        /* if goto */
        struct { Operand *t1; relop_kind relop; Operand *t2, *label; } triop;
        /* dec */
        struct { Operand *op; int size; } dec;
    } u;
    list_head list;
};
typedef struct InterCode InterCode;

/* code */
Operand* new_label();
Operand* new_op_var();
#define new_temp() new_op_tvar()
Operand* new_op_tvar();
Operand* new_op_func(char *);
Operand* new_op_cons(int);
Operand* new_op_vaddr(Operand *);
Operand* new_op_tvaddr(Operand *);
InterCode* new_ic(ir_kind);

static void export_args(Operand *arg) {}
void export_code(InterCode *c);
void delete_code(InterCode *c);
void print_code(list_head *code, FILE *fp);
void print_op(Operand *op, FILE *fp);

void translate_exp(tnode *, Operand *);
void translate_cond(tnode *, Operand *, Operand *);
Operand* translate_args(tnode *);

/* optimize */

#endif
