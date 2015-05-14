#ifndef __TREE_PARSE_H__
#define __TREE_PARSE_H__
#include "list.h"

enum bool {
    true = 1, false = 0
};
typedef enum bool bool;

enum _type_id_ {
    _int_, _float_, _array_, _struct_
};
typedef enum _type_id_ _type_id_;

struct type_t {
    _type_id_ tid;
    union {
        struct {
            char *name;
            list_head struct_domain_symbol;
            list_head struct_list;
        };
        struct {
            int size;
            struct type_t *ta; // for array
        };
    };
};
typedef struct type_t type_t;
#define tid_of(x) (x)->tid
#define type_int(x) ((x)->tid == _int_)
#define type_float(x) ((x)->tid == _int_)
#define type_struct(x) ((x)->tid == _int_)
#define type_array(x) ((x)->tid == _int_)

#define type_array_size(x) ((x)->size)
#define type_array_next(x) ((x)->ta)
#define type_struct_name(x) ((x)->name)

struct func_mes {
    type_t *rett;
    int argc;
    list_head argv_list;
    //int vis_tag;
};
typedef struct func_mes func_mes;

struct var_mes {
    type_t *type;
};
typedef struct var_mes var_mes;

struct symbol {
    list_head list;
    char *name;
    // var or fun
    bool is_var;
    int line;
    union {
        func_mes *fmes;
        var_mes *vmes;
    };
};
typedef struct symbol symbol;

struct hash_t {
    int hash;
    list_head hash_list;
    list_head symbol_list;
};
typedef struct hash_t hash_t;

struct sstack {
    list_head stack_list;
    list_head hash_list;
};
typedef struct sstack sstack;

extern list_head sstack_root;
extern list_head tstruct_root;

#define ERR_VARI_UNDEF      1
#define ERR_FUNC_UNDEF      2
#define ERR_VARI_REDEF      3
#define ERR_FUNC_REDEF      4
#define ERR_TYPE_NOTEQ      5
#define ERR_CAN_NOT_ASSIGN  6
#define ERR_OP_MISMATCH     7
#define ERR_RET_MISMATCH    8
#define ERR_F_PARA_MISMATCH 9
#define ERR_ARRAY_ACCESS    10
#define ERR_FUNC_ACCESS     11
#define ERR_A_OFF_NOT_INT   12
#define ERR_DOT_ON_NOSTR    13
#define ERR_DOMAIN_FAIL     14
#define ERR_STR_DEF_FAIL    15
#define ERR_STR_REDEF       16
#define ERR_STR_UNDEF       17
#define ERR_FUNC_NODEF      18
#define ERR_FUNC_REDEC      19

#endif

/*
 * if one struct has no name, name = NULL
 * and export it's symbol to higher level
 * */

