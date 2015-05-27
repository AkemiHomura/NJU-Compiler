#include <stdio.h>
#include <stdlib.h>
#include "irgen.h"
#include "ta.h"
#include "tree.h"

list_head code;

Operand zero = {CONSTANT, .u.cons = 0, NULL};
Operand one = {CONSTANT, .u.cons = 1, NULL};

void init_irgen() {
    list_init(&code);
}

void export_code(InterCode *c) {
    list_init(&c->list);
    list_add_before(&code, &c->list);
}

void delete_code(InterCode *c) {
    list_del(&c->list);
}

void print_code(list_head *code, FILE *fp) {

}

void print_op(Operand *op, FILE *fp) {

}

int gen_no(op_kind op) {
    static int t = 0, l = 0, v = 0;
    switch(op) {
        case VARIABLE:
            return v ++;
        case TEMPVAR:
            return t ++;
        case LABEL:
            return l ++;
        default:
            return -1;
    }
}

Operand* new_label() {
    Operand *nl = new(Operand);
    nl->kind = LABEL;
    nl->u.label = new(Label);
    nl->u.label->no = gen_no(LABEL);
    nl->u.label->next = NULL;
    nl->next = NULL;
    return nl;
}

Operand* new_op_var() {
    Operand *v = new(Operand);
    v->kind = VARIABLE;
    v->u.var_no = gen_no(VARIABLE);
    v->next = NULL;
    return v;
}

Operand* new_op_tvar() { Operand *tv = new(Operand);
    tv->kind = TEMPVAR;
    tv->u.var_no = gen_no(TEMPVAR);
    tv->next = NULL;
    return tv;
}

Operand* new_op_func(char *fname) {
    Operand *f = new(Operand);
    f->kind = FUNCTION;
    f->u.value = fname;
    f->next = NULL;
    return f;
}

Operand* new_op_cons(int cons) {
    Operand *c = new(Operand);
    c->kind = CONSTANT;
    c->u.cons = cons;
    c->next = NULL;
    return c;
}

Operand* new_op_vaddr(Operand *var) {
    Operand *vad = new(Operand);
    vad->kind = VADDRESS;
    vad->u.name = var;
    vad->next = NULL;
    return vad;
}

Operand* new_op_tvaddr(Operand *tvar) {
    Operand *tvad = new(Operand);
    tvad->kind = TADDRESS;
    tvad->u.name = tvar;
    tvad->next = NULL;
    return tvad;
}

InterCode* new_ic(ir_kind irk) {
    InterCode *ic = new(InterCode);
    ic->kind = irk;
    list_init(&ic->list);
    return ic;
}

void translate_exp(tnode *exp, Operand *place) {
    assert(label_equal(exp, Exp));
    symbol *s;
    switch(label_of(exp->son)) {
        case _INT_: {
            Operand *op = new_op_cons(exp->son->intval);
            InterCode *ic = new_ic(IR_ASSIGN);
            ic->u.assign.left = place;
            ic->u.assign.right = op;
            export_code(ic);
        }
            break;
        case _ID_:
            s = find_by_name_global(id_str(exp->son));
            if(label_equal(exp->last_son, ID)) {
                if(!s->op) {
                    Operand *op = new_op_var();
                    s->op = op;
                }
                InterCode *ic = new_ic(IR_ASSIGN);
                ic->u.assign.left = place;
                ic->u.assign.right = s->op;
                export_code(ic);
            } else {

                if(!s->op) s->op = new_op_func(s->name);
                Operand *op = s->op;
                if(label_equal(tnode_thi_son(exp), Args)) {
                    Operand *args = translate_args(tnode_thi_son(exp));
                    if(strcmp(s->name, "write") == 0) {
                        InterCode *ic = new(InterCode, IR_WRITE,
                                .u.one.op = args);
                        export_code(ic);
                    } else {
                        InterCode *ic;
                        while(args) {
                            ic = new(InterCode, IR_ARG,
                                    .u.one.op = args);
                            export_code(ic);
                            args = args->next;
                        }
                        ic = new(InterCode, IR_FUNCTION,
                                .u.assign.left = place,
                                .u.assign.right = op);
                        export_code(ic);
                    }
                } else {
                    if(strcmp(s->name, "read") == 0) {
                        InterCode *ic = new(InterCode, IR_READ,
                            .u.one.op = op);
                        export_code(ic);
                    } else {
                        InterCode *ic = new(InterCode, IR_CALL,
                                .u.assign.left = place,
                                .u.assign.right = op);
                        export_code(ic);
                    }
                }
            }
            break;
        case _MINUS_: {
            Operand *t = new_temp();
            translate_exp(exp->last_son, t);
            InterCode *ic = new_ic(IR_SUB);
            ic->u.binop.result = place;
            ic->u.binop.op1 = &zero;
            ic->u.binop.op2 = t;
            export_code(ic);
        }
            break;
        case _Exp_: {
            switch(label_of(tnode_sec_son(exp))) {
                case _ASSIGNOP_: {
                    s = find_by_name_global(id_str(exp->son->son));
                    Operand *t = new_temp();
                    translate_exp(exp->last_son, t);
                    if(!s->op) {
                        Operand *op = new_op_var();
                        s->op = op;
                    }
                    InterCode *ic = new_ic(IR_ASSIGN);
                    ic->u.assign.left = s->op;
                    ic->u.assign.right = t;
                    export_code(ic);
                    ic = new_ic(IR_ASSIGN);
                    ic->u.assign.left = place;
                    ic->u.assign.right = s->op;
                    export_code(ic);
                }
                    break;
                case _PLUS_:
                case _MINUS_:
                case _STAR_:
                case _DIV_: {
                    Operand *t1 = new_temp();
                    Operand *t2 = new_temp();
                    translate_exp(exp->son, t1);
                    translate_exp(exp->last_son, t2);
                    InterCode *ic;
                    if(label_equal(tnode_sec_son(exp), PLUS))
                        ic = new_ic(IR_ADD);
                    else if(label_equal(tnode_sec_son(exp), MINUS))
                        ic = new_ic(IR_SUB);
                    else if(label_equal(tnode_sec_son(exp), STAR))
                        ic = new_ic(IR_MUL);
                    else ic = new_ic(IR_DIV);
                    ic->u.binop.result = place;
                    ic->u.binop.op1 = t1;
                    ic->u.binop.op2 = t2;
                    export_code(ic);
                }
                    break;
                case _RELOP_:
                case _AND_:
                case  _OR_: {
                    Operand *label1 = new_label();
                    Operand *label2 = new_label();
                    InterCode *ic = new(InterCode, IR_ASSIGN,
                            .u.assign.left = place, .u.assign.right = &zero);
                    export_code(ic);
                    translate_cond(exp, label1, label2);
                    ic = new(InterCode, IR_LABEL, .u.one.op = label1);
                    export_code(ic);
                    ic = new(InterCode, IR_ASSIGN,
                            .u.assign.left = place, .u.assign.right = &one);
                    export_code(ic);
                }
                    break;
                default: break;
            }
        }
            break;
        case _NOT_: {
            Operand *label1 = new_label();
            Operand *label2 = new_label();
            InterCode *ic = new(InterCode, IR_ASSIGN,
                    .u.assign.left = place, .u.assign.right = &zero);
            export_code(ic);
            translate_cond(exp, label1, label2);
            ic = new(InterCode, IR_LABEL, .u.one.op = label1);
            export_code(ic);
            ic = new(InterCode, IR_ASSIGN,
                    .u.assign.left = place, .u.assign.right = &one);
            export_code(ic);
        }
            break;
        default: break;
    }
}

void translate_cond(tnode *exp, Operand *label_true, Operand *label_false) {
    assert(label_equal(exp, Exp));
    switch(label_of(exp->son)) {
        case _NOT_: {
            translate_cond(exp->son, label_false, label_true);
        }
            break;
        case _Exp_: {
            switch(label_of(tnode_sec_son(exp))) {
                case _RELOP_: {
                    Operand *t1 = new_temp();
                    Operand *t2 = new_temp();
                    translate_exp(exp->son, t1);
                    translate_exp(exp->last_son, t2);
                    InterCode *ic = new(InterCode, IR_IFGOTO,
                            .u.triop.t1 = t1, .u.triop.t2 = t2,
                            .u.triop.relop = tnode_sec_son(exp)->rk,
                            .u.triop.label = label_true);
                    export_code(ic);
                    ic = new(InterCode, IR_GOTO, .u.one.op = label_false);
                    export_code(ic);
                }
                    break;
                case _AND_: {
                    Operand *t = new_label();
                    translate_cond(exp->son, t, label_false);
                    InterCode *ic = new(InterCode, IR_LABEL, .u.one.op = t);
                    export_code(ic);
                    translate_cond(exp->last_son, label_true, label_false);
                }
                    break;
                case _OR_: {
                    Operand *t = new_label();
                    translate_cond(exp->son, label_true, t);
                    InterCode *ic = new(InterCode, IR_LABEL, .u.one.op = t);
                    export_code(ic);
                    translate_cond(exp->last_son, label_true, label_false);
                }

                    break;
                default: {
                    Operand *t = new_temp();
                    translate_exp(exp, t);
                    InterCode *ic = new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                            .u.triop.relop = NE, .u.triop.t2 = &zero,
                            .u.triop.label = label_true);
                    export_code(ic);
                    ic = new(InterCode, IR_GOTO, .u.one.op = label_false);
                    export_code(ic);
                }
                    break;
            }
        }
            break;
        default: {
            Operand *t = new_temp();
            translate_exp(exp, t);
            InterCode *ic = new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                    .u.triop.relop = NE, .u.triop.t2 = &zero,
                    .u.triop.label = label_true);
            export_code(ic);
            ic = new(InterCode, IR_GOTO, .u.one.op = label_false);
            export_code(ic);
        }
            break;
    }
}

Operand *translate_args(tnode *args) {
    assert(label_equal(args, Args));
    Operand *t = new_temp();
    translate_exp(args->son, t);
    if(label_equal(args->last_son, Args)) {
        t->next = translate_args(args->last_son);
    }
    return t;
}

