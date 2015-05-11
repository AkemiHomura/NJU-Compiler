#include "ta.h"
#include "tree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

list_head stack_root; list_head hash_root;

tnode *tree_ptr;

type_t type_int = {_int, 0};
type_t type_float = {_float, 0};
type_t type_void = {_void, 0};

void print_semantic_error(int err, int lineno, char *statement) {
    fprintf(stderr, "Error type [%d] at line %d: %s\n",
            err, lineno, statement);
}

static unsigned hash_pjw(char *name) {
    unsigned val = 0, i;
    for(; *name; ++ name) {
        val = (val << 2) + *name;
        if(i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

void init_symbol_table() {
    list_init(&stack_root);
    list_init(&hash_root);
}

struct func_mes* new_function_message(_type ret_type, int argc,
        type_t **argv) {
    func_mes *fm = (func_mes *)malloc(sizeof(func_mes));
    fm->ret_type = ret_type;
    fm->argc = argc;
    fm->argv = (type_t **)malloc(sizeof(type_t *) * argc);
    int i = 0;
    for(; i < argc; i ++)
        fm->argv[i] = argv[i];

    return fm;
}

struct var_mes* new_varible_message(type_t *arg) {
    var_mes *vm = (var_mes *)malloc(sizeof(var_mes));
    vm->type = arg;

    return vm;
}

struct symbol* create_symbol(char *name, int func_or_variable,
        void *mes, int lineno) {
    struct symbol *s = (struct symbol*)malloc(sizeof(struct symbol));
    list_init(&s->col_list);
    list_init(&s->row_list);

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

struct type_t* new_struct_type() {
    type_t *t = (type_t *)malloc(sizeof(type_t));
    t->tname = _struct;
    t->sec_lv = NULL;
    t->sam_lv = NULL;
    t->size = 0;

    return t;
}

struct type_t* new_array_type(int size) {
    type_t *t = (type_t *)malloc(sizeof(type_t));
    t->tname = _array;
    t->sec_lv = NULL;
    t->sam_lv = NULL;
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

struct stack_ptr* new_stack_ptr() {
    stack_ptr *sp = (stack_ptr *)malloc(sizeof(stack_ptr));
    list_init(&sp->stack_list);
    list_init(&sp->symbol_list);

    return sp;
}

void push_stack() {

}

void pop_stack() {

}

void link_hash_table(list_head *hash_root, hash_table *ht) {
    list_add_before(hash_root, &ht->hash_list);
}

void link_symbol_to_hash_table(symbol *s, hash_table *ht) {
    list_add_before(&ht->hash_list, &s->row_list);
}

void del_symbol_from_hash_table(symbol *s) {
    list_del(&s->row_list);
}

#define EQUAL_BY_NAME   1
#define EQUAL_BY_DOMAIN 2

/* equal_flag as above, return when
 * equal : 1
 * not equal : 0
 */

#define var_type_sec_level(x) ((x)->sec_lv->variable_message->type)
#define var_type_sam_level(x) ((x)->sam_lv->variable_message->type)

int multi_struct_equal(type_t *a, type_t *b) {
    if(a == NULL && b == NULL) return 1;
    else if(!(a != NULL && b != NULL)) return 0;

    if(a->tname != b->tname) return 0;

    int ret = 1;
    if(!(a->sec_lv != NULL && b->sec_lv != NULL))
        return 0;
    else {
        ret =  multi_struct_equal(
                var_type_sec_level(a),
                var_type_sec_level(b)
                );
    }

    if(!ret) return 0;

    type_t *at = var_type_sam_level(a), *bt = var_type_sam_level(b);

    while(at != NULL && bt != NULL) {
        ret &= multi_struct_equal(at, bt);
        at = var_type_sam_level(at);
        bt = var_type_sam_level(bt);
    }

    return ret;
}

int type_equal(symbol *a, symbol *b, int equal_flag) {
    int is_equal;
    switch(equal_flag) {
        case EQUAL_BY_DOMAIN:
            {
                /* Only Called by Structure Analysis
                 * */
                assert(a->func_or_variable == 1 && b->func_or_variable == 1);
                type_t *at, *bt;
                at = a->variable_message->type;
                bt = b->variable_message->type;
                assert(at->tname == _struct && bt->tname == _struct);

                is_equal = multi_struct_equal(at, bt);
            }
            break;
        case EQUAL_BY_NAME:
        default:
            if(a->name, b->name) is_equal = 1;
            else is_equal = 0;
            break;
    }

    return is_equal;
}

void func_def(tnode** t) {

}

void var_def(tnode** t) {

}

void assign(tnode** t) {

}

void array(tnode** t) {

}

void struct_domain(tnode** t) {

}

void analyse_exp(tnode **t) {

}

void func_dec(tnode **t) {

}

extern tnode* root;

void main_parser() {
    tree_ptr = root;
}



