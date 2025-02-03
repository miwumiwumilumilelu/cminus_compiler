#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定义语法树节点结构
#define MAXCHILD 10
struct Ast {
    int line;
    char *name;
    int n; // 子节点个数
    union {
        char* type;
        int i;
        float f;
    };
    struct Ast* child[MAXCHILD]; // 指向子节点的链表
};

// 定义符号表结构
typedef struct Symbol {
    char *name;
    char *type;
    int is_function;
    struct Symbol *next;
} Symbol;

typedef struct SymTab {
    Symbol *head;
} SymTab;

// 全局变量
extern int yyparse(void);
extern int yylineno;
extern char *yytext;
extern FILE *yyin;
struct Ast *root = NULL; // 语法树的根节点

// 函数声明
struct Ast* newLeaf(char* s, int yyline);
struct Ast *newNode(char *s, int yyline, int num, struct Ast* arr[]);
struct Ast *newNode0(char *s, int num);
void printTree(struct Ast* r, int layer);
SymTab* createSymTab();
void insertSymbol(SymTab *symtab, char *name, char *type, int is_function);
Symbol* lookupSymbol(SymTab *symtab, char *name);
void printSymTab(SymTab *symtab);
void semanticAnalysis(struct Ast *root, SymTab *symtab);

// 创建语法树叶子节点
struct Ast* newLeaf(char* s, int yyline) {
    struct Ast *l = (struct Ast *)malloc(sizeof(struct Ast));
    l->name = s; // 语法单元名字
    l->line = yyline;
    l->n = 0;
    if ((!strcmp(l->name, "ID")) || (!strcmp(l->name, "TYPE"))) {
        char *t = (char *)malloc(sizeof(char) * 10);
        strcpy(t, yytext);
        l->type = t;
    } else if (!strcmp(l->name, "INT")) {
        l->i = atoi(yytext);
    } else if (!strcmp(l->name, "FLOAT")) {
        l->f = atof(yytext);
    }
    return l;
}

// 创建语法树分支节点
struct Ast *newNode(char *s, int yyline, int num, struct Ast* arr[]) {
    struct Ast *l = (struct Ast *)malloc(sizeof(struct Ast));
    l->name = s;
    l->line = yyline;
    l->n = num;
    for (int i = 0; i < l->n; i++) {
        l->child[i] = arr[i];
    }
    return l;
}

// 创建空的语法树节点
struct Ast *newNode0(char *s, int num) {
    struct Ast *l = (struct Ast *)malloc(sizeof(struct Ast));
    l->name = s;
    l->n = 0;
    l->line = -1;
    return l;
}

// 打印语法树
void printTree(struct Ast* r, int layer) {
    if (r != NULL && r->line != -1) {
        for (int i = 0; i < layer; i++) {
            printf("  ");
        }
        printf("%s", r->name); // 语法单元名字
        if ((!strcmp(r->name, "ID")) || (!strcmp(r->name, "TYPE"))) {
            printf(": %s", r->type);
        } else if (!strcmp(r->name, "INT")) {
            printf(": %d", r->i);
        } else if (!strcmp(r->name, "FLOAT")) {
            printf(": %f", r->f);
        } else if (r->n != 0) { // 语法单元输出行号
            printf(" (%d)", r->line);
        }
        printf("\n");
        for (int k = 0; k < r->n; k++) {
            printTree(r->child[k], layer + 1);
        }
    }
}

// 创建符号表
SymTab* createSymTab() {
    SymTab *symtab = (SymTab*)malloc(sizeof(SymTab));
    symtab->head = NULL;
    return symtab;
}

// 插入符号到符号表
void insertSymbol(SymTab *symtab, char *name, char *type, int is_function) {
    Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = strdup(type);
    symbol->is_function = is_function;
    symbol->next = symtab->head;
    symtab->head = symbol;
}

// 查找符号
Symbol* lookupSymbol(SymTab *symtab, char *name) {
    Symbol *current = symtab->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// 打印符号表
void printSymTab(SymTab *symtab) {
    Symbol *current = symtab->head;
    while (current != NULL) {
        printf("Name: %s, Type: %s, Function: %d\n", current->name, current->type, current->is_function);
        current = current->next;
    }
}

// 语义分析
void semanticAnalysis(struct Ast *root, SymTab *symtab) {
    if (root == NULL) return;

    if (strcmp(root->name, "Program") == 0) {
        // 遍历所有外部定义
        for (int i = 0; i < root->n; i++) {
            semanticAnalysis(root->child[i], symtab);
        }
    } else if (strcmp(root->name, "ExtDef") == 0) {
        // 处理外部定义
        if (root->n == 3 && strcmp(root->child[1]->name, "FunDec") == 0) {
            // 函数定义
            char *funcName = root->child[1]->child[0]->type;
            insertSymbol(symtab, funcName, "function", 1);
        }
    } else if (strcmp(root->name, "VarDec") == 0) {
        // 变量声明
        char *varName = root->child[0]->type;
        insertSymbol(symtab, varName, "int", 0); // 假设所有变量都是int类型
    } else if (strcmp(root->name, "Exp") == 0) {
        // 表达式
        if (root->n == 3 && strcmp(root->child[1]->name, "ASSIGNOP") == 0) {
            // 赋值操作
            char *varName = root->child[0]->type;
            Symbol *symbol = lookupSymbol(symtab, varName);
            if (symbol == NULL) {
                printf("Error: Variable '%s' not declared.\n", varName);
            }
        }
    }

    // 递归处理子节点
    for (int i = 0; i < root->n; i++) {
        semanticAnalysis(root->child[i], symtab);
    }
}

// 主函数
int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    yyin = f;
    yyparse();

    // 创建符号表并进行语义分析
    SymTab *symtab = createSymTab();
    semanticAnalysis(root, symtab);

    // 打印符号表
    printSymTab(symtab);

    return 0;
}
