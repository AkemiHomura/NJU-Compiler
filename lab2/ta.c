#include "ta.h"
#include "tree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

type_t std_type_int = {_int_, NULL, NULL, NULL};
type_t std_type_float = {_float_, NULL, NULL, NULL};

list_head sstack_root;

void pserror(int err, int line, char *errinfo,
        ...) {
    fprintf(stderr, "Error type %d at line %d: ", err,
            line);
    char *format, *info;
    va_list arg_ptr;
    va_start(arg_ptr, errinfo);
    vfprintf(stderr, errinfo, arg_ptr);
    va_end(arg_ptr);
    fprintf(stderr, ".\n");
}

static unsigned hash_pjw(char *name) {
    unsigned val = 0, i;
    for(; *name; ++ name) {
        val = (val << 2) + *name;
        if((i = val & ~0x3fff)) val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

static void init_parse() {
    list_init(&sstack_root);
    push_sstack();
}

func_mes* new_func(type_t *rett) {
    func_mes *fm = (func_mes *)malloc(sizeof(func_mes));
    fm->rett = rett;
    fm->argc = 0;
    fm->vis_tag = 0;
    list_init(&fm->argv_list);
    return fm;
}

var_mes* new_var(type_t *type) {
    var_mes *vm = (var_mes *)malloc(sizeof(var_mes));
    vm->type =type;
    return vm;
}

symbol* new_symbol(char *name, bool is_var, int line, void *mes) {
    symbol *s = (symbol *)malloc(sizeof(symbol));
    list_init(&s->list);
    s->name = name;
    s->is_var = is_var;
    s->line = line;
    s->fmes = mes;
    return s;
}

type_t* new_type_struct(char *name) {
    type_t *t = (type_t *)malloc(sizeof(type_t));
    tid_of(t) = _struct_;
    t->name = name;
    list_init(&t->struct_domain_symbol);
    list_init(&t->struct_list);
    return t;
}

type_t* new_type_array(int size) {
    type_t *t = (type_t *)malloc(sizeof(type_t));
    tid_of(t) = _array_;
    t->size = size;
    return t;
}

static hash_t* new_hash_table(int hash) {
    hash_t *ht = (hash_t *)malloc(sizeof(hash_t));
    ht->hash = hash;
    list_init(&ht->hash_list);
    list_init(&ht->symbol_list);
    return ht;
}

static sstack* new_sstack() {
    sstack *sst = (sstack *)malloc(sizeof(sstack));
    list_init(&sst->stack_list);
    list_init(&sst->hash_list);
    list_init(&sst->struct_list);
    return sst;
}

static void link_sstack(sstack *sst) {
    list_add_after(&sstack_root, &sst->stack_list);
}

void push_sstack() {
    sstack *sst = new_sstack();
    link_sstack(sst);
}

void pop_sstack() {
    assert(!list_empty(&sstack_root));
    list_del(sstack_root.next);
}

static void link_hash_table_to_stack(sstack *sst,  hash_t *ht) {
    list_add_before(&sst->hash_list, &ht->hash_list);
}

hash_t* find_by_hash_in_stack(sstack *sst, int hash) {
    list_head *p;
    hash_t *ret;
    list_foreach(p, &sst->hash_list) {
        ret = list_entry(p, hash_t, hash_list);
        if(ret->hash == hash) return ret;
    }
    return NULL;
}

hash_t* find_by_hash_global(int hash) {
    list_head *ptr;
    hash_t* ret = NULL;
    list_foreach(ptr, &sstack_root) {
        sstack *sst = get_sstack(ptr);
        ret = find_by_hash_in_stack(sst, hash);
        if(ret) break;
    }
    return ret;
}

symbol* find_by_name_in_stack(sstack *sst, char *name) {
    int hash = hash_pjw(name);
    hash_t *ht = find_by_hash_in_stack(sst, hash);
    if(ht == NULL) return NULL;
    list_head *p;
    symbol *s;
    list_foreach(p, &ht->symbol_list) {
        s = list_entry(p, symbol, list);
        if(strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

symbol* find_by_name_global(char *name) {
    list_head *ptr;
    symbol* ret = NULL;
    list_foreach(ptr, &sstack_root) {
        sstack *sst = get_sstack(ptr);
        ret = find_by_name_in_stack(sst, name);
        if(ret) break;
    }
    return ret;
}

symbol* copy_symbol(symbol *src) {
    symbol *s = (symbol *)malloc(sizeof(symbol));
    memcpy(s, src, sizeof(symbol));
    list_init(&s->list);
    return s;
}

void link_symbol_to_hash_table(symbol *s, hash_t *ht) {
    list_add_before(&ht->symbol_list, &s->list);
}

void export_symbol_to_stack(symbol *s, sstack *sst) {
    int hash = hash_pjw(s->name);
    hash_t *ht = find_by_hash_in_stack(sst, hash);
    if(!ht) {
        ht = new_hash_table(hash);
        link_hash_table_to_stack(sst, ht);
    }
    link_symbol_to_hash_table(s, ht);
}

void export_symbol_to_func(symbol *s, func_mes *fm) {
    list_add_before(&fm->argv_list, &s->list);
    fm->argc ++;
}

void export_symbol_to_struct(symbol *s, type_t *type) {
    assert(type_struct(type));
    list_add_before(&type->struct_domain_symbol, &s->list);
}

void export_func_arg_to_sstack(func_mes *fm, sstack *sst) {
    list_head *ptr;
    symbol *s;
    assert(list_empty(&sst->hash_list));
    list_foreach(ptr, &fm->argv_list) {
        s = list_entry(ptr, symbol, list);
        export_symbol_to_stack(copy_symbol(s), sst);
    }
}

void export_type_struct(type_t *type) {
    assert(type_struct(type));
    list_add_before(&sstack_top->struct_list, &type->struct_list);
}

type_t *get_struct(char *sname) {
    type_t *ret;
    sstack *sst;
    for(sst = sstack_top; ; sst = sstack_down(sst)) {
        if((ret = get_struct_in_sstack(sname ,sst)))
            return ret;
        if(sst == sstack_bottom) break;
    }
    return NULL;
}

type_t *get_struct_in_sstack(char *sname, sstack *sst) {
    list_head *p;
    type_t *ret;
    list_foreach(p, &sst->struct_list) {
        ret = list_entry(p, type_t, struct_list);
        if(strcmp(ret->name, sname) == 0) return ret;
    }
    return NULL;
}

bool domain_of_struct(type_t *st, char *dname, type_t **dtype) {
    assert(type_struct(st));
    list_head *p;
    symbol *s;
    bool ret = false;
    list_foreach(p, &st->struct_domain_symbol) {
        s = list_entry(p, symbol, list);
        assert(s->is_var);
//        printf("%s %s\n", s->name, dname);
        if(strcmp(s->name ,dname) == 0) {
            if(dtype != NULL) *dtype = s->vmes->type;
            return true;
        }
        if(type_struct(s->vmes->type) && strcmp(s->name, "$") == 0)
            ret =  domain_of_struct(s->vmes->type, dname, dtype);
        if(ret) return true;
    }
    return false;
}

bool func_arg_dup(func_mes *fm, char *aname) {
    list_head *ptr;
    symbol *args;
    list_foreach(ptr, &fm->argv_list) {
        args = list_entry(ptr, symbol, list);
        if(strcmp(args->name, aname) == 0) return true;
    }
    return false;
}

bool type_equal(type_t *a, type_t *b) {
//    printf("type equal %d %d\n", a->tid, b->tid);
    if(a == NULL || b == NULL) return false;
    if(tid_of(a) != tid_of(b)) return false;
    if(type_struct(a)) {
        list_head *ap, *bp;
        symbol *sa, *sb;
        for(ap = a->struct_domain_symbol.next, bp = b->struct_domain_symbol.next;
                ap != &a->struct_domain_symbol && bp != &b->struct_domain_symbol;
                ap = ap->next, bp = bp->next) {
            sa = list_entry(ap, symbol, list);
            sb = list_entry(bp, symbol, list);
            assert(sa->is_var && sb->is_var);
            if(!type_equal(sa->vmes->type, sb->vmes->type)) return false;
        }
        if(!(ap == &a->struct_domain_symbol && bp == &b->struct_domain_symbol)) return false;
        return true;
    } else if(type_array(a)) {
        return type_equal(a->ta, b->ta);
    } else return true;
}

bool func_arg_check(func_mes *fm, type_t *type, int mode) {
    static symbol *arg_s;
    static int argc, argct;
    switch(mode) {
        case FUNC_ARG_CHECK_INIT:
            argc = 0;
            argct = fm->argc;
            arg_s = list_entry(fm->argv_list.next, symbol, list);
            return false;
        case FUNC_ARG_CHECK_GO:
            argc ++;
            if(argc > argct) return false;
            {
                int ret = type_equal(arg_s->vmes->type, type);
                if(argc < argct) arg_s = list_entry(arg_s->list.next,
                        symbol, list);
                return ret;
            }
        case FUNC_ARG_CHECK_END:
            if(argc == argct) return true;
            else return false;
        default: return false;
    }
}

void func_def_check() {
    list_head *p, *q;
    list_foreach(p, &sstack_bottom->hash_list) {
        hash_t *ht = list_entry(p, hash_t, hash_list);
        list_foreach(q, &ht->symbol_list) {
            symbol *s = list_entry(q, symbol, list);
            if(!s->is_var && !s->fmes->vis_tag) {
                pserror(ERR_FUNC_NODEF, s->line,
                        "Undefined function \"%s\"", s->name);
            }
        }
    }
}

bool func_equal(func_mes *a, func_mes *b) {
    if(!type_equal(a->rett, b->rett)) return false;
    if(a->argc != b->argc) return false;
    list_head *p, *q;
    symbol *s, *t;
    for(p = a->argv_list.next, q = b->argv_list.next;
            p != &a->argv_list; p = p->next, q = q->next) {
        s = list_entry(p, symbol, list);
        t = list_entry(q, symbol, list);
        if(!type_equal(s->vmes->type, t->vmes->type))
            return false;
    }
    return true;
}


/* parse part;
 *
 *
 */

type_t* struct_specifier(tnode *t) {
    assert(label_equal(t, StructSpecifier));
    type_t *st = NULL;
    if(label_equal(tnode_sec_son(t), Tag)) {
        tnode *id = tnode_sec_son(t)->son;
        assert(label_equal(id, ID));
        st = get_struct(id_str(id));
        if(!st) pserror(ERR_STR_UNDEF, t->line,
                "Undefined structure \"%s\"", id_str(id));
    } else {
        tnode *opt_tag = tnode_sec_son(t);
        tnode *id = opt_tag->son;
        if (!label_equal(opt_tag, OptTag)) {
            st = new_type_struct("$");
        } else {
            assert(label_equal(id, ID));
            if(!get_struct_in_sstack_top(id_str(id))) {
                st = new_type_struct(id_str(id));
            } else goto Duplicated;
        }
        export_type_struct(st);
        def_list_in_struct(tnode_for_son(t), st);
        return st;
Duplicated:
        pserror(ERR_STR_REDEF, id->line,
                "Duplicated name \"%s\"", id_str(id));
        return NULL;
    }
    return st;
}

type_t* specifier(tnode *t) {
    assert(label_equal(t, Specifier));
    type_t *ret = NULL;
    if(label_equal(t->son, TYPE)) {
        if(t->son->intval == 0)
            ret = &std_type_int;
        else
            ret = &std_type_float;
    } else {
        ret = struct_specifier(t->son);
    }
    return ret;
}

void gen_func_arg(tnode *vl, func_mes *fm) {
    if(label_equal(vl, VarList)) {
        tnode *pd = vl->son;
        symbol *args = NULL;
        while(label_equal(pd, ParamDec)) {
            type_t *argt = specifier(pd->son);
            if(argt != NULL)
                args = var_dec(pd->last_son, argt);
            if(func_arg_dup(fm, args->name))
                pserror(ERR_VARI_REDEF, pd->line,
                        "Redefined varible \"%s\"", args->name);
            else
                export_symbol_to_func(args, fm);
            vl = vl->last_son;
            pd = vl->son;
        }
    }
}

void check_func_dec(tnode *vl, symbol *fs) {
    func_mes *fm = fs->fmes;
    func_arg_check_init(fm);
    if(label_equal(vl, VarList)) {
        tnode *pd = vl->son;
        while(label_equal(pd, ParamDec)) {
            type_t *argt = specifier(pd->son);
            if(argt == NULL || !func_arg_check_go(argt)) {
                pserror(ERR_FUNC_REDEC, fs->line,
                        "Inconsistent declaration of function \"%s\"",
                        fs->name);
                break;
            }
            vl = vl->last_son;
            pd = vl->son;
        }
    }
}

void fun_dec(tnode *t, type_t *rett) {
    assert(label_equal(t, FunDec));
    tnode *id = t->son;
    symbol *fs = NULL;
    func_mes *fm = NULL;
    bool redef = false , _def = false;
    fs = find_by_name_global(id_str(id));
    tnode *vl = tnode_thi_son(t);
    if(label_equal(t->brother, CompSt)) _def = true;
    if(_def) {
        if(fs) {
            if(fs->is_var || fs->fmes->vis_tag) {
                pserror(ERR_FUNC_REDEF, t->line,
                        "Redefined function \"%s\"", id_str(id));
                redef = true;
            }
            if(!fs->is_var) check_func_dec(vl, fs);
        }
        fm = new_func(rett);
        gen_func_arg(vl, fm);
        if(!fs) {
            fs = new_symbol(id_str(id), false, t->line, fm);
            export_symbol(fs);
        }
        fs->fmes->vis_tag = 1;
        compst(t->brother, rett, fm);
    } else {
        if(fs) {
            if(fs->is_var || fs->fmes->vis_tag) {
                pserror(ERR_FUNC_REDEF, t->line,
                        "Redefined function \"%s\"", id_str(id));
            }
            if(!fs->is_var) check_func_dec(vl, fs);
        } else {
            fm = new_func(rett);
            gen_func_arg(vl ,fm);
            fs = new_symbol(id_str(id), false, t->line, fm);
            export_symbol(fs);
        }
    }
}

static type_t* var_dec_array(tnode *t, type_t *type) {
    assert(label_equal(t, VarDec) && label_equal(t->son, VarDec));
    type_t *at = new_type_array(tnode_thi_son(t)->intval);
    at->ta = type;
    if(label_equal(t->son->son, VarDec)) at = var_dec_array(t->son, at);
    return at;
}

symbol *var_dec(tnode *t, type_t *type) {
    assert(label_equal(t, VarDec));
    symbol *s = NULL;
    var_mes *vm;
    tnode *id = t->son;
    if(label_equal(t->son, ID)) {
        vm = new_var(type);
    } else {
        type = var_dec_array(t, type);
        vm = new_var(type);
        while(!label_equal(id, ID)) id = id->son;
    }
    s = new_symbol(id_str(id), true, t->son->line, vm);

    return s;
}

void ext_dec_list(tnode *t, type_t *type) {
    assert(label_equal(t, ExtDecList));
    symbol *s;
    while(label_equal(t, ExtDecList)) {
        s = var_dec(t->son, type);
        if(find_by_name_global(s->name))
            pserror(ERR_VARI_REDEF, s->line,
                    "Redefined variable \"%s\"", s->name);
        else
            export_symbol(s);

        t = t->last_son;
    }
}

void dec(tnode *t, type_t *type, type_t *st) {
    assert(label_equal(t, Dec));
    symbol *s;
    s = var_dec(t->son, type);
    if(!st) {// func
        if(find_by_name_in_stack_top(s->name)) {
            pserror(ERR_VARI_REDEF, s->line,
                    "Redefined variable \"%s\"", s->name);
            return;
        }
        else export_symbol(s);

        if(label_equal(t->last_son, Exp)) {
            type_t *et = expression(t->last_son);
            if(!type_equal(et, s->vmes->type))
                pserror(ERR_TYPE_NOTEQ, s->line,
                        "Type mismatched for assignment");
        }
    } else {
        if(domain_of_struct(st, s->name, NULL)) {
            pserror(ERR_STR_DEF_FAIL, s->line,
                    "Redefined field \"%s\"", s->name);
            return;
        }
        else export_symbol_to_struct(s, st);

        if(label_equal(t->last_son, Exp)) {
            pserror(ERR_STR_DEF_FAIL, s->line,
                    "Not allowed \"=\"");
            return;
        }
    }
}

void dec_list(tnode *t, type_t *type, type_t *st) {
    assert(label_equal(t, DecList));
    while(!label_equal(t, Dec)) {
        dec(t->son, type, st);
        t = t->last_son;
    }
}

void def(tnode *t, type_t *st) {
    assert(label_equal(t, Def));
    type_t *type = specifier(t->son);
    dec_list(tnode_sec_son(t), type, st);
}

void def_list(tnode *t, type_t *st) {
    assert(label_equal(t, DefList));
    while(true) {
        def(t->son, st);
        if(!tnode_sec_son(t)) break;
        t = t->last_son;
    }
}

void stmt(tnode *t, type_t *rett) {
//    printf("%d %s\n", t->syntax_label, t->info);
    assert(label_equal(t, Stmt));
    switch(label_of(t->son)) {
        case _Exp_: {
            expression(t->son);
            break;
        }
        case _CompSt_: {
            compst(t->son, rett, NULL);
            break;
        }
        case _RETURN_: {
            type_t *type = expression(tnode_sec_son(t));
            if(!type_equal(rett, type))
                pserror(ERR_RET_MISMATCH, t->line,
                        "Type mismatched for return");
            break;
        }
        case _IF_: {
            expression(tnode_thi_son(t));
            if(tnode_fiv_son(t)->brother)
                stmt(tnode_fiv_son(t), rett);
            stmt(t->last_son, rett);
            break;
        }
        case _WHILE_: {
            expression(tnode_thi_son(t));
            stmt(t->last_son, rett);
            break;
        }
        default: break;
    }
}

void stmt_list(tnode *t, type_t *rett) {
    assert(label_equal(t, StmtList));
    while(true) {
        stmt(t->son, rett);
        if(!tnode_sec_son(t)) break;
        t = t->last_son;
    }
}

void args(tnode *t, func_mes *fm) {
    assert(label_equal(t, Args));
    if(fm) func_arg_check_init(fm);
    while(label_equal(t, Args)) {
        type_t *arg_t = expression(t->son);
        func_arg_check_go(arg_t);
        t = t->last_son;
    }
    if(!func_arg_check_end())
        pserror(ERR_F_PARA_MISMATCH, t->line,
                "Inapplicable arguments for function");
}

typedef struct exp_ret {
    type_t *type;
    bool is_left;
} exp_ret;

static exp_ret __expression(tnode *t) {
    assert(label_equal(t, Exp));
    exp_ret ret, tret;
    ret.type = NULL;
    ret.is_left = false;
    symbol *s;
    switch(label_of(t->son)) {
        case _INT_:
            ret.type = &std_type_int;
            break;
        case _FLOAT_:
            ret.type = &std_type_float;
            break;
        case _ID_:
            if(label_equal(t->last_son, ID)) {
//                printf("%s\n", id_str(t->son));
                if((s = find_by_name_global(id_str(t->son))) &&
                        s->is_var) {
                    ret.is_left = true;
                    ret.type = s->vmes->type;
                }
                else
                    pserror(ERR_VARI_UNDEF, t->line,
                            "Undefined variable \"%s\"", id_str(t->son));
            } else {
                if((s = find_by_name_global(id_str(t->son)))) {
                    if(s->is_var) {
                        pserror(ERR_FUNC_ACCESS, t->line,
                                "\"%s\" is not a function", id_str(t->son));
                        break;
                    }
                    ret.type = s->fmes->rett;
                    if(label_equal(tnode_thi_son(t), Args))
                        args(tnode_thi_son(t), s->fmes);
                } else
                    pserror(ERR_FUNC_UNDEF, t->line,
                            "Undefined function \"%s\"", id_str(t->son));
            }
            break;
        case _MINUS_:
        case _NOT_:
        case _LP_:
            ret = __expression(tnode_sec_son(t));
            ret.is_left = false;
            break;
        case _Exp_:
            ret = __expression(t->son);
            switch(label_of(tnode_sec_son(t))) {
                case _ASSIGNOP_:
                    if(!ret.is_left) {
                        pserror(ERR_CAN_NOT_ASSIGN, t->son->line,
                                "The left-hand side of an assignment must be a variable");
                        break;
                    }
                    ret.is_left = false;
                    tret = __expression(t->last_son);
                    if(!type_equal(ret.type, tret.type)) {
                        pserror(ERR_TYPE_NOTEQ, t->son->line,
                                "Type mismatched for assignment");
                        break;
                    }
                    break;
                case _AND_:
                case _OR_:
                    tret = __expression(t->last_son);
                    if(ret.type != &std_type_int ||
                            tret.type != &std_type_int)
                        pserror(ERR_OP_MISMATCH, t->son->line,
                                "Type mismatched for operands");
                    ret.is_left = false;
                    break;
                case _RELOP_:
                case _PLUS_:
                case _MINUS_:
                case _STAR_:
                case _DIV_:
                    tret = __expression(t->last_son);
                    if(tret.type != ret.type || (ret.type != &std_type_int
                            && ret.type != &std_type_float))
                        pserror(ERR_OP_MISMATCH, t->son->line,
                                "Type mismatched for operands");
                    ret.is_left = false;
                    break;
                case _LB_: {
                    tnode *fexp = t->son;
                    if(!type_array(ret.type))
                        goto LB_FAIL;
                    tret = __expression(tnode_thi_son(t));
                    if(tret.type != &std_type_int) {
                        pserror(ERR_A_OFF_NOT_INT, fexp->line,
                                "Not an integer in []");
                    }
                    ret.type = ret.type->ta;
                    ret.is_left = true;
                    break;
LB_FAIL:
                    pserror(ERR_ARRAY_ACCESS, fexp->line,
                            "Not an array");
                    ret.type = NULL;
                    ret.is_left = false;
                }
                    break;
                case _DOT_: {
                    if(!type_struct(ret.type)) {
                        pserror(ERR_DOT_ON_NOSTR, t->son->line,
                                "Illegal use of \".\"");
                        goto DOT_FAIL;
                    }
                    if(!domain_of_struct(ret.type, id_str(t->last_son), &ret.type)) {
                        pserror(ERR_DOMAIN_FAIL, t->son->line,
                                "Non-existent field \"%s\"", id_str(t->last_son));
                        goto DOT_FAIL;
                    }
                    ret.is_left = true;
                    break;
DOT_FAIL:
                    ret.type = NULL;
                    ret.is_left = false;
                }
                    break;
                default: break;
            }
            break;
        default: break;
    }
    return ret;
}

type_t* expression(tnode *t) {
    return __expression(t).type;
}

void compst(tnode *t, type_t *rett, func_mes *fm) {
    push_sstack();
    export_func_arg(fm);
    if(t->snum == 3) {
        if(label_equal(tnode_sec_son(t), DefList))
            def_list_in_func(tnode_sec_son(t));
        else stmt_list(tnode_sec_son(t), rett);
    } else if(t->snum == 4) {
        def_list_in_func(tnode_sec_son(t));
        stmt_list(tnode_thi_son(t), rett);
    }
    pop_sstack();
}

void ext_def(tnode *t) {
    type_t *type = specifier(t->son);
    switch(label_of(tnode_sec_son(t))) {
        case _ExtDecList_:
            ext_dec_list(tnode_sec_son(t), type);
            break;
        case _FunDec_:
            fun_dec(tnode_sec_son(t), type);
            break;
        case _SEMI_:
        default:
            break;
    }
}

void ext_def_list(tnode *t) {
    while(true) {
        ext_def(t->son);
        if(!tnode_sec_son(t)) break;
        t = t->last_son;
    }
}

void main_parse(tnode *tp) {
    assert(label_equal(tp, Program));
    init_parse();
    ext_def_list(tp->son);
    func_def_check();
}


