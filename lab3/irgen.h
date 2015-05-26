#ifndef __IRGEN_H__
#define __IRGEN_H__
#include "list.h"
#include <stdio.h>

typedef enum {
    IR_ASSIGN, IR_ADD, IR_SUB, IR_MUL, IR_DIV, IR_RETURN,
    IR_LABEL, IR_GOTO, IR_IFGOTO, IR_READ, IR_WRITE, IR_CALL,
    IR_ARG, IR_FUNCTION, IR_PARAM, IR_DEC, IR_RIGHTAT
} ir_kind;

typedef enum {
    VARIABLE, CONSTANT, TEMPVAR, VADDRESS, LABEL,
    FUNCTION, TADDRESS
} op_kind;

struct Operand {
    op_kind kind;
    union {
        int var_no; // tempvar, labal
        int value;  // constant
        struct Operand* name; //vaddress, taddress
    } u;
    struct Operand* next;
};
typedef struct Operand Operand;

typedef enum {
    EQ, LT, LE, GT, GE, NE
} relop_kind;

struct InterCode {
    ir_kind kind;
    union {
        /* return, label, goto, read, write, arg, function, param */
        struct { Operand op; } one;
        /* assign, call */
        struct { Operand right, left; } assign;
        /* add, sub, mul, div */
        struct { Operand result, op1, op2; } binop;
        /* if goto */
        struct { Operand t1; relop_kind relop; Operand t2, label; } triop;
        /* dec */
        struct { Operand op; int size; } dec;
    } u;
    list_head list;
};
typedef struct InterCode InterCode;

struct Label {
    int no;
    struct Label *next;
};
typedef struct Label Label;

/* code */
void export_code(InterCode *c);
void delete_code(InterCode *c);
void print_code(list_head *code, FILE *fp);
void print_op(Operand *op, FILE *fp);

extern list_head code;

/* optimize */

#endif
