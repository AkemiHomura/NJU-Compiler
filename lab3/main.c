#include <stdio.h>
#include "syntax.tab.h"
#include "tree.h"
#include "irgen.h"

extern tnode* root;
extern int err;


void print_tree(tnode *);
void main_parse(tnode *);

int main(int argc, char** argv) {
    if(argc <= 1) return 1;
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    FILE *fp = stdout;
    if(argc == 3) {
        fp = fopen(argv[2], "w");
    }

    yyrestart(f);
#if YYDEBUG
    yydebug = 1;
#endif
    yyparse();
    if(!err) {
#ifdef DEBUG
        print_tree(root);
#endif
        main_parse(root);
        print_code(&code, fp);
    }
    return 0;
}

void print_node(int pre, tnode *r) {
    int i = 0;
    for(; i < pre; i ++) printf("  ");
        switch(r->syntax_label) {
            case _INT_:
        printf("%s", r->info);
                    printf(": %d\n", r->intval);
                    break;
            case _FLOAT_:
        printf("%s", r->info);
                    printf(": %f\n", r->floval);
                    break;
            case _TYPE_:
        printf("%s", r->info);
                    if(r->intval == 0) printf(": int\n");
                    else printf(": float\n");
                    break;
            case _ID_:
        printf("%s", r->info);
                    printf(": %s\n", r->strval);
                    break;
            default:
        printf("%s (%d)\n", r->info, r->line);
                    break;
        }

}

void print_tree(tnode *r) {
    if(r == NULL) return;
    static int pre = 0;
    tnode *ptr = r->son;
    print_node(pre, r);
    pre ++;
    while(ptr != NULL) {
        print_tree(ptr);
        ptr = ptr->brother;
    }
    pre --;
}
