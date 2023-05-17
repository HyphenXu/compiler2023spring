#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>

#include "ast.h"
#include "koopair.h"

using namespace std;

// TA's words:
// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

typedef enum{
    CMODE_KOOPA,
    CMODE_RISCV,
} cmode_t;

int main(int argc, const char *argv[]) {
    // TA's words:
    // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
    // compiler 模式 输入文件 -o 输出文件
    assert(argc == 5);
    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    cmode_t cmode;
    if(strcmp(mode, "-koopa") == 0){
        cmode = CMODE_KOOPA;
    }
    else if(strcmp(mode, "-riscv") == 0){
        cmode = CMODE_RISCV;
    }
    else{
        assert(false);
    }

    // TA's words:
    // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
    yyin = fopen(input, "r");
    assert(yyin);

    // TA's words:
    // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
    unique_ptr<BaseAST> ast;
    auto ret = yyparse(ast);
    assert(!ret);

    // DEBUG: dump AST
    ast->Dump();

    // dump StringFormatKoopaIR
    stringstream ss;
    ss.clear();
    ss.str("");
    streambuf* old_buffer = cout.rdbuf(ss.rdbuf());
    ast->Dump2StringIR(nullptr);
    string string_koopair(ss.str());
    cout.rdbuf(old_buffer);

    if(cmode == CMODE_KOOPA){
        FILE *fp_out = fopen(output, "w");
        fprintf(fp_out, "%s", string_koopair.c_str());
        return 0;
    }
    else
    if(cmode == CMODE_RISCV){
        ofstream fout(output);
        streambuf* old_buffer = cout.rdbuf(fout.rdbuf());
        libkoopa_string2rawprog2riscv(string_koopair.c_str());
        cout.rdbuf(old_buffer);
    }

    return 0;
}
