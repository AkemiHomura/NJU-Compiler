#ifndef __TREE_ANALYSIS_H__
#define __TREE_ANALYSIS_H__
#include <stdint.h>
#include "list.h"

enum bool {
    true = 1, false = 0
};
typedef enum bool bool;

enum _type {
    _int, _float, _array, _struct, _void
};
typedef enum _type _type;

struct type_t {
    enum _type t;
    union {
        struct {
            struct type_struct_t *in_struct;
            char *struct_name;
        };
        struct type_t *in_array;
    };
    int size;
    list_head list;
};
typedef struct type_t type_t;

struct type_struct_t {
    struct type_t *type;
    char *name;
    struct type_struct_t *brother;
};
typedef struct type_struct_t type_struct_t;

struct hash_table {
    list_head hash_list;
    list_head symbol_list;
    int hash;
};
typedef struct hash_table hash_table;

struct func_mes {
    struct type_t *ret_type;
    int argc;
    type_t** argv;
};
typedef struct func_mes func_mes;

struct var_mes {
    struct type_t *type;
};
typedef struct var_mes var_mes;

struct symbol {
    list_head list;
    char *name;
    /* function 0
     * variable 1
     * */
    int func_or_variable;
    int visited_tag;
    int lineno;
    union {
        struct func_mes *function_message;
        struct var_mes *variable_message;
    };
};
typedef struct symbol symbol;

struct stack_ptr {
    list_head stack_list;
    list_head node_list;
};
typedef struct stack_ptr stack_ptr;

struct stack_node {
    list_head *symbol_ptr;
    list_head *hash_ptr;
    list_head list;
};
typedef struct stack_node stack_node;


extern list_head stack_root;
extern list_head hash_root;
extern list_head type_root;

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
#define ERR_STR_REDEF       16  //g
#define ERR_STR_UNDEF       17  //g
#define ERR_FUNC_NODEF      18
#define ERR_FUNC_REDEC      19


#endif
