#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定义全局变量
char *src;          // 源代码指针
int line = 1;       // 行号计数器
int token;          // 当前词法单元
int token_val;      // 词法单元的值（用于数字或标识符）
char *data;         // 数据段指针（用于存储字符串）

// 符号表结构
typedef struct {
    char *name;     // 符号名称
    int hash;       // 哈希值
    int token;      // 词法单元类型
} Symbol;

Symbol symbol_table[100]; // 符号表
int symbol_ptr = 0;       // 符号表指针

// 词法单元类型定义
enum {
    Num = 128, Id, Assign, Eq, Inc, Dec, Add, Sub, Mul, Div, Mod, Lt, Gt, Le, Ge, Ne, Shl, Shr, Lor, Land, Or, And, Xor, Brak, Cond
};

// 词法分析函数
void tokenize() {
    char *ch_ptr;
    while ((token = *src++)) {
        if (token == '\n') line++;
        else if (token == '#') while (*src != 0 && *src != '\n') src++;
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
            ch_ptr = src - 1;
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_'))
                token = token * 147 + *src++;
            token = (token << 6) + (src - ch_ptr);
            symbol_ptr = 0;
            while (symbol_table[symbol_ptr].token) {
                if (token == symbol_table[symbol_ptr].hash && !memcmp(symbol_table[symbol_ptr].name, ch_ptr, src - ch_ptr)) {
                    token = symbol_table[symbol_ptr].token;
                    return;
                }
                symbol_ptr++;
            }
            symbol_table[symbol_ptr].name = ch_ptr;
            symbol_table[symbol_ptr].hash = token;
            token = symbol_table[symbol_ptr].token = Id;
            // 输出单词
            printf("Token: %.*s\n", (int)(src - ch_ptr), ch_ptr);
            return;
        }
        else if (token >= '0' && token <= '9') {
            if ((token_val = token - '0'))
                while (*src >= '0' && *src <= '9') token_val = token_val * 10 + *src++ - '0';
            else if (*src == 'x' || *src == 'X')
                while ((token = *++src) && ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')))
                    token_val = token_val * 16 + (token & 0xF) + (token >= 'A' ? 9 : 0);
            else while (*src >= '0' && *src <= '7') token_val = token_val * 8 + *src++ - '0';
            token = Num;
            printf("Token: %d\n", token_val); // 输出数字
            return;
        }
        else if (token == '"' || token == '\'') {
            ch_ptr = data;
            while (*src != 0 && *src != token) {
                if ((token_val = *src++) == '\\') {
                    if ((token_val = *src++) == 'n') token_val = '\n';
                }
                if (token == '"') *data++ = token_val;
            }
            src++;
            if (token == '"') token_val = (int)(long)ch_ptr; // 使用 long 避免指针到整数的警告
            else token = Num;
            printf("Token: %.*s\n", (int)(src - ch_ptr - 1), ch_ptr); // 输出字符串或字符
            return;
        }
        else if (token == '/') {
            if (*src == '/') {
                while (*src != 0 && *src != '\n') src++;
            } else {
                token = Div;
                printf("Token: /\n"); // 输出除号
                return;
            }
        }
        else if (token == '=') { if (*src == '=') { src++; token = Eq; printf("Token: ==\n"); } else { token = Assign; printf("Token: =\n"); } return; }
        else if (token == '+') { if (*src == '+') { src++; token = Inc; printf("Token: ++\n"); } else { token = Add; printf("Token: +\n"); } return; }
        else if (token == '-') { if (*src == '-') { src++; token = Dec; printf("Token: --\n"); } else { token = Sub; printf("Token: -\n"); } return; }
        else if (token == '!') { if (*src == '=') { src++; token = Ne; printf("Token: !=\n"); } else { printf("Token: !\n"); } return; }
        else if (token == '<') { if (*src == '=') { src++; token = Le; printf("Token: <=\n"); } else if (*src == '<') { src++; token = Shl; printf("Token: <<\n"); } else { token = Lt; printf("Token: <\n"); } return; }
        else if (token == '>') { if (*src == '=') { src++; token = Ge; printf("Token: >=\n"); } else if (*src == '>') { src++; token = Shr; printf("Token: >>\n"); } else { token = Gt; printf("Token: >\n"); } return; }
        else if (token == '|') { if (*src == '|') { src++; token = Lor; printf("Token: ||\n"); } else { token = Or; printf("Token: |\n"); } return; }
        else if (token == '&') { if (*src == '&') { src++; token = Land; printf("Token: &&\n"); } else { token = And; printf("Token: &\n"); } return; }
        else if (token == '^') { token = Xor; printf("Token: ^\n"); return; }
        else if (token == '%') { token = Mod; printf("Token: %%\n"); return; }
        else if (token == '*') { token = Mul; printf("Token: *\n"); return; }
        else if (token == '[') { token = Brak; printf("Token: [\n"); return; }
        else if (token == '?') { token = Cond; printf("Token: ?\n"); return; }
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
            printf("Token: %c\n", token); // 输出单个字符
            return;
        }
    }
}

// 主函数
int main(int argc, char *argv[]) {
    char filename[256];

    // 如果未通过命令行参数指定文件名，则提示用户输入
    if (argc < 2) {
        printf("请输入文件名: ");
        scanf("%s", filename);
    } else {
        strcpy(filename, argv[1]);
    }

    // 读取源代码文件
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("无法打开文件");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    src = (char *)malloc(file_size + 1);
    if (!src) {
        perror("内存分配失败");
        fclose(file);
        return 1;
    }
    size_t read_size = fread(src, 1, file_size, file); // 检查 fread 返回值
    if (read_size != file_size) {
        perror("读取文件失败");
        free(src);
        fclose(file);
        return 1;
    }
    src[file_size] = 0; // 添加字符串结束符
    fclose(file);

    // 初始化数据段
    data = (char *)malloc(1024);
    if (!data) {
        perror("内存分配失败");
        free(src);
        return 1;
    }

    // 词法分析
    while (*src) {
        tokenize();
    }

    // 释放内存
    free(src);
    free(data);
    return 0;
}
