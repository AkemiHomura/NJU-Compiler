#ifndef __IRGEN_H__
#define __IRGEN_H__
#include "list.h"
#include "tree.h"
#include "bool.h"
#include <stdio.h>

#define new(type, ...) ({ \
    type *__ = (type *)malloc(sizeof(type)); \
    *__ = (type){__VA_ARGS__}; \
    __; \
})
extern list_head code;

typedef enum {
    IR_ASSIGN, IR_ADD, IR_SUB, IR_MUL, IR_DIV, IR_RETURN,
    IR_LABEL, IR_GOTO, IR_IFGOTO, IR_READ, IR_WRITE, IR_CALL,
    IR_ARG, IR_FUNCTION, IR_PARAM, IR_DEC, IR_RIGHTAT, IR_LEFTSTAR,
    IR_RIGHTSTAR
} ir_kind;

typedef enum {
    VARIABLE, CONSTANT, TEMPVAR, LABEL,
    FUNCTION, VARIADDR
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
    bool is_arg;
    void *reg_ptr;
    int offset2fp;
    int size;
    struct Operand* next;
};
typedef struct Operand Operand;

struct InterCode {
    ir_kind kind;
    union {
        /* return, label, goto, read, write, arg, function, param */
        struct { Operand *op; } one;
        /* assign, call, leftstar, rightstar, rightat */
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
InterCode* new_ic(ir_kind);

void export_code(InterCode *c);
InterCode* get_last_code();
bool last_is_temp_assign_cons(Operand *temp);
bool last_is_temp_assign_vari(Operand *temp);
void delete_last_code();

void delete_code(InterCode *c);
void print_code(list_head *code, FILE *fp);
void print_op(Operand *op, FILE *fp);

void translate_exp(tnode *, Operand *);
void translate_cond(tnode *, Operand *, Operand *);
Operand* translate_args(tnode *);

/* optimize */
struct basic_block {
    list_head *begin;
    list_head *end;
    list_head list;
};
typedef struct basic_block basic_block;
void divide_into_basic_blocks(list_head *code, basic_block *root);
/* begin is one before true begin */
void optimize_basic_block(basic_block *bc);

extern Operand zero, one;

#endif
