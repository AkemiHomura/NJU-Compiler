#include "ta.h"
#include "tree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define label_of(ptr) ((ptr)->syntax_label)
list_head stack_root, hash_root, type_root;

type_t type_int = {_int, NULL};
type_t type_float = {_float, NULL};
type_t type_void = {_void, NULL};

void print_semantic_error(int err, int lineno, char *statement,
        char *var_name) {
    fprintf(stderr, "Error type [%d] at line %d: %s \"%s\"\n",
            err, lineno, statement, var_name);
}

static unsigned hash_pjw(char *name) {
    unsigned val = 0, i;
    for(; *name; ++ name) {
        val = (val << 2) + *name;
        if((i = val & ~0x3fff)) val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

void init_symbol_table() {
    list_init(&stack_root);
    list_init(&hash_root);
    list_init(&type_root);

    list_add_before(&type_root, &type_int.list);
    list_add_before(&type_root, &type_float.list);
    list_add_before(&type_root, &type_void.list);
}

/* just new,
 * argc, argv not filled
 * */
struct func_mes* new_function_message(type_t *ret_type) {
    func_mes *fm = (func_mes *)malloc(sizeof(func_mes));
    fm->ret_type = ret_type;
    list_init(&fm->argv_list);

    return fm;
}

struct var_mes* new_varible_message(type_t *type) {
    var_mes *vm = (var_mes *)malloc(sizeof(var_mes));
    vm->type = type;

    return vm;
}

struct symbol* new_symbol(char *name, int func_or_variable,
        int lineno, void *mes) {
    struct symbol *s = (struct symbol*)malloc(sizeof(struct symbol));
    list_init(&s->list);

    s->name = (char *)malloc(strlen(name) + 1);
    strcpy(s->name, name);
    s->func_or_variable = func_or_variable;
    s->visited_tag = 0;
    s->lineno = lineno;
    if(func_or_variable)
        memcpy(s->function_message, mes, sizeof(func_mes));
    else
        memcpy(s->variable_message, mes, sizeof(var_mes));

    return s;
}

static inline void touch_symbol(symbol* s) {
    s->visited_tag ++;
}

struct type_t* new_struct_type(char *name) {
    type_t *t = (type_t *)malloc(sizeof(type_t));
    t->t = _struct;
    if(name != NULL)
        t->struct_name = (char *)malloc(strlen(name) + 1);
    else t->struct_name = NULL;
    strcpy(t->struct_name, name);
    t->size = 0;

    return t;
}

struct type_t* new_array_type(int size) {
    type_t *t = (type_t *)malloc(sizeof(type_t));
    t->t = _array;
    t->size = size;

    return t;
}

struct hash_table *new_hash_table(int hash) {
    hash_table *ht = (hash_table *)malloc(sizeof(hash_table));
    list_init(&ht->hash_list);
    list_init(&ht->symbol_list);

    ht->hash = hash;

    return ht;
}

void link_hash_table(list_head *hash_root, hash_table *ht) {
    list_add_before(hash_root, &ht->hash_list);
}

hash_table* find_table_by_hash(int hash) {
    list_head *ptr;
    hash_table *ret;
    list_foreach(ptr, &hash_root) {
        ret = list_entry(ptr, hash_table, hash_list);
        if(ret->hash == hash) break;
    }

    if(ptr != &hash_root) return ret;
    else return NULL;
}

symbol* find_symbol_by_name(char *name) {
    int hash = hash_pjw(name);
    hash_table *ht = find_table_by_hash(hash);
    if(ht == NULL) return NULL;
    list_head *ptr;
    symbol *s;
    list_foreach_rev(ptr, &ht->symbol_list) {
        s = list_entry(ptr, symbol, list);
        if(strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

symbol* find_symbol_by_name_in_last_stack(char *name) {
    if(list_empty(&stack_root)) return find_symbol_by_name(name);
    int hash = hash_pjw(name);
    hash_table *ht = find_table_by_hash(hash);
    if(ht == NULL) return NULL;
    list_head *ptr;
    symbol *s;
    list_foreach(ptr, &(list_entry(stack_root.prev, stack_ptr,
                    stack_list)->node_list)) {
        stack_node *sn = list_entry(ptr, stack_node, list);
        if(&ht->hash_list == sn->hash_ptr) {
            list_head *p;
            for(p = sn->symbol_ptr->next; p != sn->hash_ptr;
                    p = p->next) {
                s = list_entry(p, symbol, list);
                if(strcmp(s->name, name) == 0) return s;
            }
            break;
        }
    }
    return NULL;
}

void link_symbol_to_hash_table(symbol *s, hash_table *ht) {
    list_add_before(&ht->symbol_list, &s->list);
}

void export_symbol(symbol *s) {
    int hash = hash_pjw(s->name);
    hash_table *ht = find_table_by_hash(hash);
    link_symbol_to_hash_table(s, ht);
}

void export_farg_symbol(symbol *s, func_mes *fm) {
    list_add_before(&fm->argv_list, &s->list);
}

void del_symbol_from_hash_table(symbol *s) {
    list_del(&s->list);
}

type_t* find_struct(char *struct_name) {
    list_head *ptr;
    list_foreach(ptr, &type_root) {
        type_t *t = list_entry(ptr, type_t, list);
        if(t->t == _struct)
            if(strcmp(t->struct_name, struct_name) == 0)
                return t;
    }

    return NULL;
}

int find_domain_in_struct(type_t *st) {
    assert(st->t == _struct);

}

void link_type(type_t *t) {
    list_add_before(&type_root, &t->list);
}

void del_type(type_t *t) {
    list_del(&t->list);
}

#define EQUAL_BY_NAME   1
#define EQUAL_BY_DOMAIN 2

/* equal_flag as above, return when
 * equal : 1
 * not equal : 0
 */

int struct_equal(type_t *, type_t *);

int array_equal(type_t *a, type_t *b) {
    assert(a->t == _array && b->t == _array);

    a = a->in_array;
    b = b->in_array;
    while(1) {
        if(a->t != b->t) return 0;
        if(a->t == _struct) {
            if(!(struct_equal(a, b)))
                return 0;
        } else if(a->t == _array) {
            a = a->in_array;
            b = b->in_array;
            continue;
        } else break;
    }
    return 1;
}

int struct_equal(type_t *a, type_t *b) {
    assert(a->t == _struct && b->t == _struct);
    type_struct_t *a_ptr, *b_ptr;
    a_ptr = a->in_struct;
    b_ptr = b->in_struct;

    while(a_ptr != NULL && b_ptr != NULL) {
        if(a_ptr->type->t != b_ptr->type->t) return 0;
        if(a_ptr->type->t == _array) {
            if(!array_equal(a_ptr->type, b_ptr->type))
                return 0;
        }
        if(a_ptr->type->t == _struct) {
            if(!struct_equal(a_ptr->type, b_ptr->type))
                return 0;
        }

        a_ptr = a_ptr->next;
        b_ptr = b_ptr->next;
    }

    if(a_ptr != NULL || b_ptr != NULL) return 0;
    return 1;
}

int type_equal(type_t *a, type_t *b, int equal_flag) {
    int is_equal;
    if(a->t != b->t) return 0;
    switch(equal_flag) {
        case EQUAL_BY_DOMAIN:
            {
                /* Only Called by Structure Analysis
                 * */
                if(a->t == _struct) {
                    is_equal = struct_equal(a, b);
                    break;
                } else if(a->t == _array) {
                    is_equal = array_equal(a, b);
                }
            }
        case EQUAL_BY_NAME:
        default:
            if(a->t == b->t) is_equal = 1;
            break;
    }

    return is_equal;
}

int check_arg_type(symbol *s, int argn, type_t *t) {
    assert(s->func_or_variable == 0);
    func_mes *fm = s->function_message;
    assert(argn < fm->argc);
    list_head *ptr;

    static int argi = -1;
    static symbol *quick_s = NULL;
    static symbol *quick_arg_s = NULL;
    if(quick_s != s) quick_s = s, argi = -1, quick_arg_s = NULL;
    else if(argn < argi) argi = -1, quick_arg_s = NULL;

    if(quick_arg_s != NULL) {
        for(ptr = quick_arg_s->list.next; argi < argn; argi ++,
                ptr = ptr->next);
    } else {
        for(ptr = fm->argv_list.next; argi < argn; argi ++,
                ptr = ptr->next);
    }
    symbol *s_arg = list_entry(ptr, symbol, list);
    assert(s_arg->func_or_variable == 1);
    quick_arg_s = s_arg;
    if(!type_equal(s_arg->variable_message->type, t, EQUAL_BY_DOMAIN))
        return 0;
    else return 1;
}

void push_stack();
void pop_stack();

/* mode :
 * 0: normal mode, link every symbol to table
 * 1: struct mode, link brother...
 * */
void __def_list(tnode *t, int mode, void *ret);
#define def_list(t) __def_list(t, 0, NULL)
#define def_list_in_struct(t, x) __def_list(t, 1, x);
#define get_id_str(t) (assert(label_of(t) == _ID_), (t)->strval)

type_t* struct_specifier(tnode *t, int link_to_type) {
    assert(label_of(t) == _StructSpecifier_);

    type_t *st = NULL;
    if(label_of(t->last_son) != _RC_) {
        /* declaration of struct */
        tnode *tag = t->last_son;
        assert(label_of(tag->son) == _ID_);
        st = find_struct(get_id_str(tag->son));
        if(!st) print_semantic_error(ERR_STR_UNDEF, t->line,
                "Undefined Struct", get_id_str(tag->son));
    } else {
        /* definition */
        tnode *opt_tag = t->son->brother;
        assert(opt_tag == NULL || label_of(opt_tag) == _OptTag_);
        if(opt_tag == NULL)
            st = new_struct_type(NULL);
        else {
            assert(label_of(opt_tag->son) == _ID_);
            if(!(st = find_struct(opt_tag->son->strval))) {
                symbol *s;
                if((s = find_symbol_by_name(get_id_str(opt_tag->son))))
                    print_semantic_error(ERR_STR_REDEF, t->line,
                            "Struct name conflict with", get_id_str(opt_tag->son));
                else {
                    st = new_struct_type(opt_tag->son->strval);
                }
            }
            else
                print_semantic_error(ERR_STR_REDEF, t->line,
                        "Struct name conflict with", opt_tag->son->strval);

        }
        if(link_to_type) link_type(st);
        if(st == NULL) return NULL;

        tnode *dl = opt_tag->brother->brother;
        assert(label_of(dl) == _DefList_);
        type_struct_t *in_s;
        def_list_in_struct(dl, &in_s);
        st->in_struct = in_s;
    }

    return st;
}

/* this function will link new type in type list
 * or find an old one in the list */
type_t* specifier(tnode *t, int link_to_type) {
    assert(label_of(t) == _Specifier_);
    type_t *ret = NULL;
    if(label_of(t->son) == _TYPE_) {
        if(label_of(t->son->son) == _INT_)
            ret = &type_int;
        else if(label_of(t->son->son) == _FLOAT_)
            ret = &type_float;
        else assert(0);
    } else {
        ret = struct_specifier(t->son, link_to_type);
    }

    return ret;
}

symbol* var_dec(tnode *t ,type_t *type);
stack_ptr *func_dec(tnode* t, type_t *ret_type) {
    assert(label_of(t) == _FunDec_);
    int hash = hash_pjw(get_id_str(t->son));
    symbol *s;
    stack_ptr *sp;
    if(find_symbol_by_name(get_id_str(t->son))) {
        print_semantic_error(ERR_FUNC_REDEF, t->son->line,
                "Function Redefinition", get_id_str(t->son));
        func_mes *fm = new_function_message(ret_type);
        s = new_symbol("$", 0, t->son->line,
                fm);
    }
    else {
        func_mes *fm = new_function_message(ret_type);
        s = new_symbol(get_id_str(t->son), 0, t->son->line,
                fm);
    }

    tnode *vl = t->son->son->son;
    /* var list */
    tnode *pd;
    if(label_of(vl) == _VarList_) {
        do{
            pd = vl->son;
            type_t *arg_type = specifier(pd->son, false);
            symbol *arg_s = var_dec(pd->last_son, arg_type);

            assert(arg_s != NULL);
            export_farg_symbol(arg_s, s->function_message);

            if(label_of(vl->last_son) != _VarList_) break;
            vl = vl->last_son;
        } while(true);
    }

    //touch_symbol(s);
    push_stack();
    return sp;
}

static type_t* var_dec_array(tnode *t, type_t *type) {
    assert(label_of(t) == _VarDec_ && label_of(t->son) == _VarDec_);
    type_t *at = new_array_type(t->son->brother->brother->brother->intval);
    at->in_array = type;
    if(label_of(t->son->son) == _VarDec_) at = var_dec_array(t->son, at);
    return at;
}

symbol* var_dec(tnode *t, type_t *type) {
    assert(label_of(t) == _VarDec_);
    symbol *s;
    if(label_of(t->son) == _ID_) {
        if((s = find_symbol_by_name_in_last_stack(get_id_str(t->son)))) {
            print_semantic_error(ERR_VARI_REDEF, t->line,
                    "Redefined variable", get_id_str(t->son));
            return NULL;
        }
        var_mes *vm = new_varible_message(type);
        s = new_symbol(get_id_str(t->son), 1,
                t->son->line, vm);
    } else {
        type = var_dec_array(t, type);
        var_mes *vm = new_varible_message(type);
        s = new_symbol(get_id_str(t->son), 1,
                t->son->line, vm);
    }

    return s;
}

void ext_dec_list(tnode *t, type_t *type) {
    assert(label_of(t) == _ExtDecList_);
    symbol *s;
    while(label_of(t) != _VarDec_) {
        s = var_dec(t->son, type);

        if(!s) export_symbol(s);
        t = t->last_son;
    }
}

type_t* expression(tnode *t);

type_struct_t *var_dec_in_struct(tnode *t, type_t *type) {
    assert(label_of(t) == _VarDec_);
    type_struct_t *ts = (type_struct_t *)malloc(sizeof(type_struct_t));
    if(label_of(t->son) != _ID_) type = var_dec_array(t, type);
    while(label_of(t) != _ID_) t = t->son;
    ts->name = (char*)malloc(strlen(get_id_str(t)) + 1);
    strcpy(ts->name, get_id_str(t));
    ts->type = type;
    ts->prev = ts->next = NULL;

    return ts;
}

void dec(tnode *t, type_t *type) {
    var_dec(t->son, type);
    if(label_of(t->last_son) == _Exp_)
        expression(t->last_son);
}

type_struct_t* dec_in_struct(tnode *t, type_t *type) {
    type_struct_t *ts = var_dec_in_struct(t, type);
    if(label_of(t->last_son) == _Exp_)
        print_semantic_error(ERR_STR_DEF_FAIL, t->line,
                "Illegal assign in struct definition", ts->name);

    return ts;
}

void dec_list(tnode *t, type_t *type) {
    assert(label_of(t) == _DecList_);
    while(label_of(t) != _Dec_) {
        dec(t->son, type);
        t = t->last_son;
    }
}

type_struct_t* dec_list_in_struct(tnode *t, type_t *type) {
    assert(label_of(t) == _DecList_);
    type_struct_t *ts, *ts_prev;
    ts = ts_prev = NULL;
    while(label_of(t) != _Dec_) {
        ts = dec_in_struct(t->son, type);
        if(!ts_prev) {
            ts_prev->next = ts;
            ts->prev = ts_prev;
        }
        ts_prev = ts;
        t = t->last_son;
    }

    return ts;
}

void def(tnode *t) {
    assert(label_of(t) == _Def_);
    type_t *type = specifier(t->son, false);

    dec_list(t->son->brother, type);
}

type_struct_t* def_in_struct(tnode *t) {
    assert(label_of(t) == _Def_);
    type_t *type = specifier(t->son, false);

    return dec_list_in_struct(t->son->brother, type);
}

inline type_struct_t *head_of(type_struct_t *ts) {
    while(ts->prev != NULL) ts = ts->prev;
    return ts;
}

/* mode :
 * 0: normal mode, link every symbol to table
 * 1: struct mode, link brother...
 * */
void __def_list(tnode *t, int mode, void *ret) {
    assert(label_of(t) == _DefList_);
    if(mode == 0) {
        while(t != NULL) {
            def(t->son);
            t = t->last_son;
        }
    } else {
        type_struct_t *ts, *ts_prev_end = NULL, *ts_next_head,
                      *ts_head;
        while(t != NULL) {
            ts = def_in_struct(t->son);
            ts_next_head = head_of(ts);
            if(!ts_prev_end) {
                ts_prev_end->next = ts_next_head;
                ts_next_head->prev = ts_prev_end;
            } else ts_head = ts_next_head;
            ts_prev_end = ts;

            t = t->last_son;
        }
        *(type_struct_t **)ret = ts_head;
    }
}

void compst(tnode *t, type_t *ret_type);
void stmt(tnode *t, type_t *ret_type) {
    switch(label_of(t->son)) {
        case _Exp_: {
            expression(t->son);
            break;
        }
        case _CompSt_: {
            compst(t->son, ret_type);
            break;
        }
        case _RETURN_: {
            type_t *ret = expression(t->son->brother);
            if(!type_equal(ret, ret_type, EQUAL_BY_DOMAIN)) {
                print_semantic_error(ERR_RET_MISMATCH, t->son->brother->line,
                        "Type mismatched for return", NULL);
            }
            break;
        }
        case _IF_: {
            expression(t->son->brother->brother);
            if(t->snum == 4) {
                stmt(t->last_son, ret_type);
            } else {
                stmt(t->last_son, ret_type);
            }
            break;
        }
        case _WHILE_: {
            expression(t->son->brother->brother);
            stmt(t->last_son, ret_type);
            break;
        }
    }
}

void stmt_list(tnode *t, type_t *ret_type) {
    assert(label_of(t) == _StmtList_);
    while(t != NULL) {
        stmt(t->son, ret_type);
        t = t->last_son;
    }
}

void args(tnode *t, symbol *func) {
    assert(label_of(t) == _Args_);
    type_t *type;
    int i = 0;
    while(label_of(t->last_son) == _Args_) {
        type = expression(t->son);
        if(!check_arg_type(func, i, type)) {
            print_semantic_error(ERR_F_PARA_MISMATCH,
                    t->son->line, "Function arguments error", func->name);
            break;
        }
    }
    type = expression(t);
    if(!check_arg_type(func, i, type)) {
        print_semantic_error(ERR_F_PARA_MISMATCH,
                t->son->line, "Function arguments error", func->name);
    }
}

type_t* expression(tnode *t) {
    symbol *s;
    if(label_of(t->son) == _INT_) return &type_int;
    else if(label_of(t->son) == _FLOAT_) return &type_float;
    else if(label_of(t->son) == _ID_) {
        if(label_of(t->last_son) == _ID_) {
            if(!(s = find_symbol_by_name(t->son->strval)))
                goto VARI_DEF_NOT_FOUND;
            if(s->func_or_variable == 0)
                goto VARI_DEF_NOT_FOUND;
            return s->variable_message->type;
VARI_DEF_NOT_FOUND:
            print_semantic_error(ERR_VARI_UNDEF, t->son->line,
                    "Undefined variable", t->son->strval);
            return NULL;
        } else {
            if(!(s = find_symbol_by_name(t->son->strval)))
                goto FUNC_DEF_NOT_FOUND;
            if(s->func_or_variable == 1)
                goto FUNC_DEF_NOT_FOUND;
            if(label_of(t->son->brother->brother) == _Args_)
                args(t->son->brother->brother, s);
            return s->function_message->ret_type;
FUNC_DEF_NOT_FOUND:
            print_semantic_error(ERR_FUNC_UNDEF, t->son->line,
                    "Undefined function", s->name);
            return NULL;
        }
    }
    else if(label_of(t->son) == _NOT_) {
        type_t *type = expression(t->last_son);
        if(type != &type_int) {
            print_semantic_error(ERR_OP_MISMATCH, t->son->line,
                    "Not on noint", NULL);
                return NULL;
        }
        return type;
    } else if(label_of(t->son) == _MINUS_) {
        type_t *type = expression(t->last_son);
        return type;
    } else if(label_of(t->son) == _LP_) {
        type_t *type = expression(t->son->brother);
        return type;
    } else {

    }
}

void compst(tnode *t, type_t *ret_type) {

}

extern tnode* root;

void main_parser(tnode *tree_ptr) {
    int i = 0;
    switch(tree_ptr->syntax_label) {
        case _ExtDef_: {
            tnode *spec = tree_ptr->son;
            /* if struct definition, type list will filled
             * by this function */
            type_t *type = specifier(spec, true);
            tnode *sec = spec->brother;
            if(label_of(sec) == _FunDec_) {
                stack_ptr *p = func_dec(sec, type);
                /* Compst */
                compst(sec->last_son, type);
            } else if(label_of(sec) == _ExtDecList_) {
                ext_dec_list(sec, type);
            }
        }
            break;
        case _SEMI_:
        case _COMMA_:
            break;
        case _Program_:
        case _ExtDefList_:
        default: {
            tnode *tptr = tree_ptr->son;
            while(tptr != NULL) {
                main_parser(tptr);
                tptr = tptr->brother;
            }
        }
            break;
    }
}


struct stack_ptr* new_stack_ptr() {
    stack_ptr *sp = (stack_ptr *)malloc(sizeof(stack_ptr));
    list_init(&sp->stack_list);

    return sp;
}

void push_stack() {
    stack_ptr *sp = (stack_ptr *)malloc(sizeof(stack_ptr));
    list_init(&sp->stack_list);
    list_init(&sp->node_list);

    list_add_before(&stack_root, &sp->stack_list);

    list_head *hp;
    list_foreach(hp, &hash_root) {
        stack_node *sn = (stack_node *)malloc(sizeof(stack_node));
        list_init(&sn->list);
        list_add_before(&sp->node_list, &sn->list);

        hash_table *ht = list_entry(hp, hash_table, hash_list);
        sn->symbol_ptr = ht->symbol_list.prev;
        sn->hash_ptr = &ht->symbol_list;
    }
}

void pop_stack() {
    stack_ptr *sp = list_entry(stack_root.prev, stack_ptr, stack_list);

    list_head *ptr;
    list_head *sptr;
    list_foreach(ptr, &sp->node_list) {
        stack_node *sn = list_entry(ptr, stack_node, list);

        for(sptr = sn->symbol_ptr->next; sptr != sn->hash_ptr; sptr = sptr->next) {
            list_del(sptr);
        }
    }
}

