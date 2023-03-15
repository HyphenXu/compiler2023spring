/* Options */
%code requires {
    #include <memory>
    #include <string>
    #include "ast.h"
}

%{
#include <iostream>
#include <memory>
#include <string>
#include "ast.h"

/* Necessary declarations. */
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;
%}

/** Define extra parameter for parser and error handler.
 *  With this pointer, we can store something for the AST after parsing.
 */
%parse-param { std::unique_ptr<BaseAST> &ast }

/** Define yylval, as a union.
 *  Lexer will use them.
 */
%union {
    // 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>
    // 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
    std::string *str_val;
    int int_val;
    BaseAST *ast_val;
}

/* Declare all possible types of the tokens returned by the lexer */
%token INT RETURN
%token  <str_val>    IDENT
%token  <int_val>    INT_CONST

/* Define the types of non-terminators */
%type <ast_val> FuncDef FuncType Block Stmt
%type <ast_val> Exp PrimaryExp UnaryExp
%type <int_val> Number
%type <str_val> UnaryOp

%%

/** The basic semantics here is: after CompUnit is parsed, what shall be
 *  done with the return value of parsing FuncDef, and what shall be
 *  returned to the caller of this parser, with the pre-defined
 *  <%parse-param> "ast".
 *  $1 here is 'the return value of the first symbol'
 */
CompUnit
    : FuncDef {
        auto comp_unit = make_unique<CompUnitAST>();
        comp_unit->func_def = unique_ptr<BaseAST>($1);
        ast = move(comp_unit);
    }
    ;

/** Miscs:
 *  Pay attention to the style here:
        - catch a pointer to a string with unique_ptr<string>
        - dereference and concatenate
        - pass the pointer to the newly constructed string
 *  $$ is the return value of the non-terminators.
 */
FuncDef
    : FuncType IDENT '(' ')' Block {
        auto ast = new FuncDefAST();
        ast->func_type = unique_ptr<BaseAST>($1);
        ast->ident = *unique_ptr<string>($2);
        ast->block = unique_ptr<BaseAST>($5);
        $$ = ast;
    }
    ;

FuncType
    : INT {
        auto ast = new FuncTypeAST();
        ast->type_string = string("int");
        $$ = ast;
    }
    ;

Block
    : '{' Stmt '}' {
        auto ast = new BlockAST();
        ast->stmt = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    ;

Stmt
    : RETURN Exp ';' {
        auto ast = new StmtAST();
        ast->exp = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    ;

Exp
    : UnaryExp {
        auto ast = new ExpAST();
        ast->unaryexp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

PrimaryExp
    : '(' Exp ')' {
        auto ast = new PrimaryExpAST();
        ast->rule = 1;
        ast->exp = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    | Number {
        auto ast = new PrimaryExpAST();
        ast->rule = 2;
        ast->number = $1;
        $$ = ast;
    }
    ;

Number
    : INT_CONST {
        $$ = $1;
    }
    ;

UnaryExp
    : PrimaryExp {
        auto ast = new UnaryExpAST();
        ast->rule = 1;
        ast->primaryexp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | UnaryOp UnaryExp {
        auto ast = new UnaryExpAST();
        ast->rule = 2;
        ast->unaryop = *unique_ptr<string>($1);
        ast->unaryexp = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    ;

UnaryOp
    : '+' { auto op = new string("+"); $$ = op; }
    | '-' { auto op = new string("-"); $$ = op; }
    | '!' { auto op = new string("!"); $$ = op; }
    ;

%%

/** Define the error handler.
 *  Parser will call this function
 *  upon error (e.g., syntax error), where the second argument
 *  is error message.
 */
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
    cerr << "error: " << s << endl;
}