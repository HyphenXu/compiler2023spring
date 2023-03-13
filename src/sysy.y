/* Options */
%code requires {
    #include <memory>
    #include <string>
}

%{
#include <iostream>
#include <memory>
#include <string>

/* Necessary declarations. */
int yylex();
void yyerror(std::unique_ptr<std::string> &ast, const char *s);

using namespace std;
%}

/** Define extra parameter for parser and error handler.
 *  With this pointer, we can store something for the AST after parsing.
 */
%parse-param { std::unique_ptr<std::string> &ast }

/** Define yylval, as a union.
 *  Lexer will use them.
 */
%union {
    // 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>
    // 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
    std::string *str_val;
    int int_val;
}

/* Declare all possible types of the tokens returned by the lexer */
%token  INT          RETURN
%token  <str_val>    IDENT
%token  <int_val>    INT_CONST

/* Define the types of non-terminators */
%type <str_val> FuncDef FuncType Block Stmt Number

%%

/** The basic semantics here is: after CompUnit is parsed, what shall be
 *  done with the return value of parsing FuncDef, and what shall be
 *  returned to the caller of this parser.
 *  $1 here is 'the return value of the first symbol'
 */
CompUnit
    : FuncDef {
        ast = unique_ptr<string>($1);
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
        auto type = unique_ptr<string>($1);
        auto ident = unique_ptr<string>($2);
        auto block = unique_ptr<string>($5);
        $$ = new string(*type + " " + *ident + "() " + *block);
    }
    ;


FuncType
    : INT {
        $$ = new string("int");
    }
    ;

Block
    : '{' Stmt '}' {
        auto stmt = unique_ptr<string>($2);
        $$ = new string("{ " + *stmt + " }");
    }
    ;

Stmt
    : RETURN Number ';' {
        auto number = unique_ptr<string>($2);
        $$ = new string("return " + *number + ";");
    }
    ;

Number
    : INT_CONST {
        $$ = new string(to_string($1));
    }
    ;

%%

/** Define the error handler.
 *  Parser will call this function
 *  upon error (e.g., syntax error), where the second argument
 *  is error message.
 */
void yyerror(unique_ptr<string> &ast, const char *s) {
    cerr << "error: " << s << endl;
}