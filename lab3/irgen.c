#include <stdio.h>
#include <stdlib.h>
#include "irgen.h"
#include "ta.h"
#include "tree.h"

extern Operand fall;

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

void print_relop(relop_kind rk, FILE *fp) {
    switch(rk) {
        case EQ:
            fprintf(fp, "==");
            break;
        case NE:
            fprintf(fp, "!=");
            break;
        case LT:
            fprintf(fp, "<");
            break;
        case GT:
            fprintf(fp, ">");
            break;
        case LE:
            fprintf(fp, "<=");
            break;
        case GE:
            fprintf(fp, ">=");
            break;
        default: assert(0);
    }
}

void print_code(list_head *code, FILE *fp) {
    list_head *p;
    InterCode *ic;
    list_foreach(p, code) {
        ic = list_entry(p, InterCode, list);
        switch(ic->kind) {
            case IR_ASSIGN:
                print_op(ic->u.assign.left, fp);
                fprintf(fp, " := ");
                print_op(ic->u.assign.right, fp);
                break;
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:
                print_op(ic->u.binop.result, fp);
                fprintf(fp, " := ");
                print_op(ic->u.binop.op1, fp);
                if(ic->kind == IR_ADD)
                    fprintf(fp, " + ");
                if(ic->kind == IR_SUB)
                    fprintf(fp, " - ");
                if(ic->kind == IR_MUL)
                    fprintf(fp, " * ");
                if(ic->kind == IR_DIV)
                    fprintf(fp, " / ");
                print_op(ic->u.binop.op2, fp);
                break;
            case IR_RETURN:
                fprintf(fp, "RETURN  ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_LABEL:
                fprintf(fp, "LABEL ");
                print_op(ic->u.one.op, fp);
                fprintf(fp, " :");
                break;
            case IR_GOTO:
                fprintf(fp, "GOTO ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_IFGOTO:
                fprintf(fp, "IF ");
                print_op(ic->u.triop.t1, fp);
                fprintf(fp, " ");
                print_relop(ic->u.triop.relop, fp);
                fprintf(fp, " ");
                print_op(ic->u.triop.t2, fp);
                fprintf(fp, " GOTO ");
                print_op(ic->u.triop.label, fp);
                break;
            case IR_READ:
                fprintf(fp, "READ ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_WRITE:
                fprintf(fp, "WRITE ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_CALL:
                print_op(ic->u.assign.left, fp);
                fprintf(fp, " := CALL ");
                print_op(ic->u.assign.right, fp);
                break;
            case IR_ARG:
                fprintf(fp, "ARG ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_FUNCTION:
                fprintf(fp, "FUNCTION ");
                print_op(ic->u.one.op, fp);
                fprintf(fp, " :");
                break;
            case IR_PARAM:
                fprintf(fp, "PARAM ");
                print_op(ic->u.one.op, fp);
                break;
            case IR_DEC:
                fprintf(fp, "DEC ");
                print_op(ic->u.dec.op, fp);
                fprintf(fp, " %d", ic->u.dec.size);
                break;
            case IR_RIGHTAT:
                print_op(ic->u.assign.left, fp);
                fprintf(fp, " := &");
                print_op(ic->u.assign.right, fp);
                break;
            case IR_LEFTSTAR:
                fprintf(fp, "*");
                print_op(ic->u.assign.left, fp);
                fprintf(fp, " := ");
                print_op(ic->u.assign.right, fp);
                break;
            case IR_RIGHTSTAR:
                print_op(ic->u.assign.left, fp);
                fprintf(fp, " := *");
                print_op(ic->u.assign.right, fp);
                break;
            default: assert(0);
        }
        fprintf(fp, "\n");
    }
}

void print_op(Operand *op, FILE *fp) {
    if(!op) return;
    switch(op->kind) {
        case VARIABLE:
            fprintf(fp, "v%d", op->u.var_no);
            break;
        case CONSTANT:
            fprintf(fp, "#%d", op->u.cons);
            break;
        case TEMPVAR:
            fprintf(fp, "t%d", op->u.var_no);
            break;
        case LABEL:
            fprintf(fp, "label%d", op->u.label->no);
            break;
        case FUNCTION:
            fprintf(fp, "%s", op->u.value);
            break;
        default: assert(0);
    }
}

int gen_no(op_kind op) {
    static int t = 1, l = 1, v = 1;
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

InterCode* new_ic(ir_kind irk) {
    InterCode *ic = new(InterCode);
    ic->kind = irk;
    list_init(&ic->list);
    return ic;
}

static type_t *var_type = NULL;
static bool comp_left = false;

void export_args(Operand *args) {
    if(args->next) export_args(args->next);
    export_code(new(InterCode, IR_ARG, .u.one.op = args));
}

void translate_exp(tnode *exp, Operand *place) {
    assert(label_equal(exp, Exp));
    symbol *s;
    switch(label_of(exp->son)) {
        case _INT_: {
            Operand *op = new_op_cons(exp->son->intval);
            if(!place) break;
            InterCode *ic = new_ic(IR_ASSIGN);
            ic->u.assign.left = place;
            ic->u.assign.right = op;
            export_code(ic);
        }
            break;
        case _LP_:
            translate_exp(tnode_sec_son(exp), place);
            break;
        case _ID_:
            s = find_by_name_global(id_str(exp->son));
            if(label_equal(exp->last_son, ID)) {
                assert(s->op);
                var_type = s->vmes->type;
                if(!place) break;
                InterCode *ic = new_ic(IR_ASSIGN);
                ic->u.assign.left = place;
                ic->u.assign.right = s->op;
                export_code(ic);
            } else {

                Operand *op = new_op_func(s->name);
                if(label_equal(tnode_thi_son(exp), Args)) {
                    Operand *args = translate_args(tnode_thi_son(exp));
                    if(strcmp(s->name, "write") == 0) {
                        InterCode *ic = new(InterCode, IR_WRITE,
                                .u.one.op = args);
                        export_code(ic);
                    } else {
                        InterCode *ic;
                        export_args(args);
                        if(!place) place = new_temp();
                        ic = new(InterCode, IR_CALL,
                                .u.assign.left = place,
                                .u.assign.right = op);
                        export_code(ic);
                    }
                } else {
                    if(strcmp(s->name, "read") == 0) {
                        if(!place) break;
                        InterCode *ic = new(InterCode, IR_READ,
                            .u.one.op = place);
                        export_code(ic);
                    } else {
                        if(!place) place = new_temp();
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
            if(!place) break;
            ic->u.binop.result = place;
            ic->u.binop.op1 = &zero;
            ic->u.binop.op2 = t;
            export_code(ic);
        }
            break;
        case _Exp_: {
            switch(label_of(tnode_sec_son(exp))) {
                case _ASSIGNOP_: {
                    if(label_equal(exp->son->son, ID)) {
                        s = find_by_name_global(id_str(exp->son->son));
                        Operand *t = new_temp();
                        translate_exp(exp->last_son, t);
                        if(!s->op) s->op = new_op_var();
                        InterCode *ic = new_ic(IR_ASSIGN);
                        ic->u.assign.left = s->op;
                        ic->u.assign.right = t;
                        export_code(ic);
                        if(!place) break;
                        ic = new_ic(IR_ASSIGN);
                        ic->u.assign.left = place;
                        ic->u.assign.right = s->op;
                        export_code(ic);
                    } else {
                        Operand *t1 = new_temp();
                        Operand *t2 = new_temp();
                        comp_left = true;
                        translate_exp(exp->son, t1);
                        comp_left = false;
                        translate_exp(exp->last_son, t2);
                        export_code(new(InterCode, IR_LEFTSTAR, .u.assign.left = t1,
                                    .u.assign.right = t2));
                    }
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
                    if(!place) break;
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
                    if(!place) break;
                    InterCode *ic = new(InterCode, IR_ASSIGN,
                            .u.assign.left = place, .u.assign.right = &zero);
                    export_code(ic);
                    translate_cond(exp, &fall, label1);
                    if(!place) break;
                    ic = new(InterCode, IR_ASSIGN,
                            .u.assign.left = place, .u.assign.right = &one);
                    export_code(ic);
                }
                    break;
                case _LB_: {
                    bool array_in_left = comp_left;
                    Operand *t1 = new_temp();
                    comp_left = true;
                    translate_exp(exp->son, t1);
                    comp_left = false;
                    type_t *var_type_copy = var_type;
                    Operand *type_size = new_op_cons(var_type_copy->ta->tsize);
                    Operand *t2 = new_temp();
                    translate_exp(tnode_thi_son(exp), t2);
                    export_code(new(InterCode, IR_MUL, .u.binop.result = t2,
                                .u.binop.op1 = t2, .u.binop.op2 = type_size));
                    export_code(new(InterCode, IR_ADD,
                                .u.binop.result = t1, .u.binop.op1 = t1,
                                .u.binop.op2 = t2));
                    if(!place) break;
                    if(array_in_left) {
                        export_code(new(InterCode, IR_ASSIGN, .u.assign.left = place,
                                    .u.assign.right = t1));
                    } else {
                        export_code(new(InterCode, IR_RIGHTSTAR, .u.assign.left = place,
                                    .u.assign.right = t1));
                    }
                    var_type = var_type_copy->ta;
                }
                    break;
                case _DOT_: {
                    bool struct_in_left = comp_left;
                    Operand *t1 = new_temp();
                    comp_left = true;
                    translate_exp(exp->son, t1);
                    comp_left = false;

                    type_t *struct_type = var_type;
                    /* find symbol in struct */
                    list_head *p;
                    symbol *s = NULL;
                    list_foreach(p, &struct_type->struct_domain_symbol) {
                        s = list_entry(p, symbol, list);
                        if(strcmp(s->name, id_str(exp->last_son)) == 0)
                            break;
                    }
                    Operand *t2 = new_op_cons(s->offset);
                    export_code(new(InterCode, IR_ADD, .u.binop.result = t1,
                                .u.binop.op1 = t1, .u.binop.op2 = t2));
                    if(!place) break;
                    if(struct_in_left) {
                        export_code(new(InterCode, IR_ASSIGN, .u.assign.left = place,
                                    .u.assign.right = t1));
                    } else {
                        export_code(new(InterCode, IR_RIGHTSTAR, .u.assign.left = place,
                                    .u.assign.right = t1));
                    }
                    var_type = s->vmes->type;
                }
                    break;
                default: break;
            }
        }
            break;
        case _NOT_: {
            Operand *label1 = new_label();
            if(!place) break;
            InterCode *ic = new(InterCode, IR_ASSIGN,
                    .u.assign.left = place, .u.assign.right = &zero);
            export_code(ic);
            translate_cond(exp, &fall, label1);
            if(!place) break;
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
        case _LP_: {
            translate_cond(tnode_sec_son(exp), label_true, label_false);
        }
            break;
        case _Exp_: {
            switch(label_of(tnode_sec_son(exp))) {
                case _RELOP_: {
                    Operand *t1 = new_temp();
                    Operand *t2 = new_temp();
                    translate_exp(exp->son, t1);
                    translate_exp(exp->last_son, t2);
                    if(label_true != &fall && label_false != &fall) {
                        InterCode *ic = new(InterCode, IR_IFGOTO,
                                .u.triop.t1 = t1, .u.triop.t2 = t2,
                                .u.triop.relop = tnode_sec_son(exp)->rk,
                                .u.triop.label = label_true);
                        export_code(ic);
                        ic = new(InterCode, IR_GOTO, .u.one.op = label_false);
                        export_code(ic);
                    } else if(label_true != &fall) {
                        export_code(new(InterCode, IR_IFGOTO,
                                    .u.triop.t1 = t1, .u.triop.t2 = t2,
                                    .u.triop.relop = tnode_sec_son(exp)->rk,
                                    .u.triop.label = label_true));
                    } else if(label_false != &fall) {
                        export_code(new(InterCode, IR_IFGOTO,
                                    .u.triop.t1 = t1, .u.triop.t2 = t2,
                                    .u.triop.relop = reverse_relop(tnode_sec_son(exp)->rk),
                                    .u.triop.label = label_false));
                    }
                }
                    break;
                case _AND_: {
                    Operand *t;
                    if(label_false != &fall)
                        translate_cond(exp->son, &fall, label_false);
                    else  {
                        t = new_label();
                        translate_cond(exp->son, &fall, t);
                    }
                    translate_cond(exp->last_son, label_true, label_false);
                    if(label_false == &fall)
                        export_code(new(InterCode, IR_LABEL, .u.one.op = t));
                }
                    break;
                case _OR_: {
                    Operand *t;
                    if(label_true != &fall)
                        translate_cond(exp->son, label_true, &fall);
                    else {
                        t = new_label();
                        translate_cond(exp->son, t, &fall);
                    }
                    translate_cond(exp->last_son, label_true, label_false);
                    if(label_true == &fall)
                        export_code(new(InterCode, IR_LABEL, .u.one.op = t));
                }

                    break;
                default: {
                    Operand *t = new_temp();
                    translate_exp(exp, t);
                    if(label_true != &fall && label_false != &fall) {
                        InterCode *ic = new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                                .u.triop.relop = NE, .u.triop.t2 = &zero,
                                .u.triop.label = label_true);
                        export_code(ic);
                        ic = new(InterCode, IR_GOTO, .u.one.op = label_false);
                        export_code(ic);
                    } else if(label_true != &fall) {
                        export_code(new(InterCode, IR_IFGOTO,.u.triop.t1 = t,
                                .u.triop.relop = NE, .u.triop.t2 = &zero,
                                .u.triop.label = label_true));
                    } else if(label_false != &fall) {
                        export_code(new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                                .u.triop.relop = EQ, .u.triop.t2 = &zero,
                                .u.triop.label = label_false));
                    }
                }
                    break;
            }
        }
            break;
        default: {
            Operand *t = new_temp();
            translate_exp(exp, t);
            if(label_true != &fall && label_false != &fall) {
                InterCode *ic = new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                        .u.triop.relop = NE, .u.triop.t2 = &zero,
                        .u.triop.label = label_true);
                export_code(ic);
                ic = new(InterCode, IR_GOTO, .u.one.op = label_false);
                export_code(ic);
            } else if(label_true != &fall) {
                export_code(new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                            .u.triop.relop = NE, .u.triop.t2 = &zero,
                            .u.triop.label = label_true));
            } else if(label_false != &fall) {
                export_code(new(InterCode, IR_IFGOTO, .u.triop.t1 = t,
                            .u.triop.relop = EQ, .u.triop.t2 = &zero,
                            .u.triop.label = label_false));
            }
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

void divide_into_basic_blocks(list_head *code) {

}

void optimize_basic_block(basic_block *bc) {

}
