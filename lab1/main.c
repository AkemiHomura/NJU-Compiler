#include <stdio.h>
#include "syntax.tab.h"
#include "tree.c"

extern tnode* root;
extern int err;

void print_tree(tnode *);

int main(int argc, char** argv) {
    if(argc <= 1) return 1;
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }

    yyrestart(f);
    yyparse();
    if(!err) print_tree(root);
    return 0;
}

void print_node(int pre, tnode *r) {
    int i = 0;
    for(; i < pre; i ++) printf("  ");
    if(r->line > 0) {
        printf("%s (%d)\n", r->info, r->line);
    } else {
        printf("%s", r->info);
        switch(r->line) {
            case -1:
                    printf("\n");
                    break;
            case -INT:
                     printf(": %d\n", r->intval);
                    break;
            case -FLOAT:
                    printf(": %f\n", r->floval);
                    break;
            case -TYPE:
                    if(r->intval == 0) printf(": int\n");
                    else printf(": float\n");
                    break;
            case -ID:
                    printf(": %s\n", r->strval);
                    break;
            default:
                    perror("Ni te me dou wo ne!\n");
                    break;
        }
    }

}

void print_tree(tnode *r) {
    if(r == NULL) return;
    static int pre = 0;
    int i = 0;
    print_node(pre, r);
    pre ++;
    for(; i < r->snum; i ++) print_tree(r->son[i]);
    pre --;
}
