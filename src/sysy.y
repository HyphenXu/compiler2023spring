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
#include <vector>

/* Necessary declarations. */
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

static int block_id = 1; /* Global is ZERO */
static int if_id = 0;
static int ret_id = 0;
static int l_or_exp_id = 0;
static int l_and_exp_id = 0;
static int while_id = 0;

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
%token              INT VOID CONST
%token              RETURN IF ELSE WHILE BREAK CONTINUE
%token  <str_val>   IDENT
%token  <int_val>   INT_CONST
%token  <str_val>   ORDEREDCOMPOP UNORDEREDCOMPOP LOGICAND LOGICOR

/* Define the types of non-terminators */
%type <ast_val> CompUnits CompUnit
%type <ast_val> Decl
%type <ast_val> ConstDecl BType ConstDefs ConstDef ConstInitVal
%type <ast_val> VarDecl VarDefs VarDef InitVal
%type <ast_val> FuncDef FuncType FuncFParams FuncFParam
%type <ast_val> Block BlockItems BlockItem GeneralStmt Stmt OpenStmt
%type <ast_val> Exp
%type <ast_val> LVal
%type <ast_val> PrimaryExp UnaryExp FuncRParams MulExp AddExp
%type <ast_val> RelExp EqExp LAndExp LOrExp
%type <ast_val> ConstExp
%type <int_val> Number
%type <str_val> UnaryOp

%start StartSymbol

%%

/* Part 1: CompUnit */
/** The basic semantics here is: after CompUnit is parsed, what shall be
 *  done with the return value of parsing FuncDef, and what shall be
 *  returned to the caller of this parser, with the pre-defined
 *  <%parse-param> "ast".
 *  $1 here is 'the return value of the first symbol'
 */

StartSymbol
    : CompUnits {
        auto start_symbol = make_unique<StartSymbolAST>();
        start_symbol->comp_units = unique_ptr<BaseAST>($1);
        ast = move(start_symbol);
    }
    ;

CompUnits
    : CompUnits CompUnit {
        auto ast = reinterpret_cast<CompUnitsAST *>($1);
        ast->vec_comp_units.push_back(unique_ptr<BaseAST>($2));
        $$ = ast;
    }
    | CompUnit {
        auto ast = new CompUnitsAST();
        ast->vec_comp_units.push_back(unique_ptr<BaseAST>($1));
        $$ = ast;
    }
    ;

CompUnit
    : FuncDef {
        // auto comp_unit = make_unique<CompUnitAST>();
        // comp_unit->func_def = unique_ptr<BaseAST>($1);
        // ast = move(comp_unit);
        auto ast = new CompUnitAST();
        ast->func_def = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

/* Part 2: Decl */
Decl
    : ConstDecl {
        auto ast = new DeclAST();
        ast->rule = 1;
        ast->decl = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | VarDecl {
        auto ast = new DeclAST();
        ast->rule = 2;
        ast->decl = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

ConstDecl
    : CONST BType ConstDefs ';' {
        auto ast = new ConstDeclAST();
        ast->b_type = unique_ptr<BaseAST>($2);
        ast->const_defs = unique_ptr<BaseAST>($3);
        $$ = ast;
    };

BType
    : INT {
        auto ast = new BTypeAST();
        ast->type_string = string("int");
        $$ = ast;
    }
    ;

ConstDefs
    : ConstDefs ',' ConstDef {
        auto ast = reinterpret_cast<ConstDefsAST *>($1);
        ast->vec_const_defs.push_back(unique_ptr<BaseAST>($3));
        $$ = ast;
    }
    | ConstDef {
        auto ast = new ConstDefsAST();
        ast->vec_const_defs.push_back(unique_ptr<BaseAST>($1));
        $$ = ast;
    }
    ;

ConstDef
    : IDENT '=' ConstInitVal {
        auto ast = new ConstDefAST();
        ast->ident = *unique_ptr<string>($1);
        ast->const_init_val = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

ConstInitVal
    : ConstExp {
        auto ast = new ConstInitValAST();
        ast->const_exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

VarDecl
    : BType VarDefs ';' {
        auto ast = new VarDeclAST();
        ast->b_type = unique_ptr<BaseAST>($1);
        ast->var_defs = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    ;

VarDefs
    : VarDefs ',' VarDef {
        auto ast = reinterpret_cast<VarDefsAST *>($1);
        ast->vec_var_defs.push_back(unique_ptr<BaseAST>($3));
        $$ = ast;
    }
    | VarDef {
        auto ast = new VarDefsAST();
        ast->vec_var_defs.push_back(unique_ptr<BaseAST>($1));
        $$ = ast;
    }
    ;

VarDef
    : IDENT {
        auto ast = new VarDefAST();
        ast->ident = *unique_ptr<string>($1);
        ast->init_val = nullptr;
        $$ = ast;
    }
    | IDENT '=' InitVal {
        auto ast = new VarDefAST();
        ast->ident = *unique_ptr<string>($1);
        ast->init_val = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

InitVal
    : Exp {
        auto ast = new InitValAST();
        ast->exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

/* Part 3: Func */
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
        ast->func_f_params = nullptr;
        ast->block = unique_ptr<BaseAST>($5);
        $$ = ast;
    }
    | FuncType IDENT '(' FuncFParams ')' Block {
        auto ast = new FuncDefAST();
        ast->func_type = unique_ptr<BaseAST>($1);
        ast->ident = *unique_ptr<string>($2);
        ast->func_f_params = unique_ptr<BaseAST>($4);
        ast->block = unique_ptr<BaseAST>($6);
        $$ = ast;
    }
    ;

FuncType
    : INT {
        auto ast = new FuncTypeAST();
        ast->type_string = string("int");
        $$ = ast;
    }
    | VOID {
        auto ast = new FuncTypeAST();
        ast->type_string = string("void");
        $$ = ast;
    }
    ;

FuncFParams
    : FuncFParams ',' FuncFParam {
        auto ast = reinterpret_cast<FuncFParamsAST *>($1);
        ast->vec_func_f_params.push_back(unique_ptr<BaseAST>($3));
        $$ = ast;
    }
    | FuncFParam {
        auto ast = new FuncFParamsAST();
        ast->vec_func_f_params.push_back(unique_ptr<BaseAST>($1));
        $$ = ast;
    }
    ;

FuncFParam
    : BType IDENT {
        auto ast = new FuncFParamAST();
        ast->b_type = unique_ptr<BaseAST>($1);
        ast->ident = *unique_ptr<string>($2);
        $$ = ast;
    }
    ;

/* Part 4: Block */
Block
    : '{' BlockItems '}' {
        auto ast = new BlockAST();
        ast->block_items = unique_ptr<BaseAST>($2);
        ast->id = block_id++;
        $$ = ast;
    }
    | '{' '}' {
        auto ast = new BlockAST();
        ast->block_items = nullptr;
        ast->id = block_id++;
        $$ = ast;
    }
    ;

BlockItems
    : BlockItems BlockItem {
        auto ast = reinterpret_cast<BlockItemsAST *>($1);
        ast->vec_block_items.push_back(unique_ptr<BaseAST>($2));
        $$ = ast;
    }
    | BlockItem {
        auto ast = new BlockItemsAST();
        ast->vec_block_items.push_back(unique_ptr<BaseAST>($1));
        $$ = ast;
    }
    ;

BlockItem
    : Decl {
        auto ast = new BlockItemAST();
        ast->is_stmt = false;
        ast->item = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | GeneralStmt {
        auto ast = new BlockItemAST();
        ast->is_stmt = true;
        ast->item = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

GeneralStmt
    : Stmt {
        auto ast = new GeneralStmtAST();
        ast->is_open = false;
        ast->stmt = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | OpenStmt {
        auto ast = new GeneralStmtAST();
        ast->is_open = false;
        ast->stmt = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

Stmt
    : LVal '=' Exp ';' {
        auto ast = new StmtAST();
        ast->rule = 1;
        ast->l_val = unique_ptr<BaseAST>($1);
        ast->exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | Exp ';' {
        auto ast = new StmtAST();
        ast->rule = 2;
        ast->exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | ';' {
        auto ast = new StmtAST();
        ast->rule = 2;
        ast->exp = nullptr;
        $$ = ast;
    }
    | Block {
        auto ast = new StmtAST();
        ast->rule = 3;
        ast->block = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | RETURN Exp ';' {
        auto ast = new StmtAST();
        ast->rule = 4;
        ast->ret_id = ret_id++;
        ast->exp = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    | RETURN ';' {
        auto ast = new StmtAST();
        ast->rule = 4;
        ast->ret_id = ret_id++;
        ast->exp = nullptr;
        $$ = ast;
    }
    | IF '(' Exp ')' Stmt ELSE Stmt {
        auto ast = new StmtAST();
        ast->rule = 5;
        ast->if_stmt_id = if_id++;
        ast->exp = unique_ptr<BaseAST>($3);
        ast->stmt_true = unique_ptr<BaseAST>($5);
        ast->stmt_false = unique_ptr<BaseAST>($7);
        $$ = ast;
    }
    | WHILE '(' Exp ')' GeneralStmt {
        auto ast = new StmtAST();
        ast->rule = 6;
        ast->while_id = while_id++;
        ast->exp = unique_ptr<BaseAST>($3);
        ast->stmt_body = unique_ptr<BaseAST>($5);
        $$ = ast;
    }
    | BREAK ';' {
        auto ast = new StmtAST();
        ast->rule = 7;
        $$ = ast;
    }
    | CONTINUE ';' {
        auto ast = new StmtAST();
        ast->rule = 8;
        $$ = ast;
    }
    ;

OpenStmt
    : IF '(' Exp ')' GeneralStmt {
        auto ast = new OpenStmtAST();
        ast->if_stmt_id = if_id++;
        ast->is_with_else = false;
        ast->exp = unique_ptr<BaseAST>($3);
        ast->stmt_true = unique_ptr<BaseAST>($5);
        $$ = ast;
    }
    | IF '(' Exp ')' Stmt ELSE OpenStmt {
        auto ast = new OpenStmtAST();
        ast->if_stmt_id = if_id++;
        ast->is_with_else = true;
        ast->exp = unique_ptr<BaseAST>($3);
        ast->stmt_true = unique_ptr<BaseAST>($5);
        ast->stmt_false = unique_ptr<BaseAST>($7);
        $$ = ast;
    }
    ;

/* Part 5: Exp */
Exp
    : LOrExp {
        auto ast = new ExpAST();
        ast->lorexp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

LVal
    : IDENT {
        auto ast = new LValAST();
        ast->ident = *unique_ptr<string>($1);
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
    | LVal {
        auto ast = new PrimaryExpAST();
        ast->rule = 3;
        ast->l_val = unique_ptr<BaseAST>($1);
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
    | IDENT '(' ')' {
        auto ast = new UnaryExpAST();
        ast->rule = 3;
        ast->ident = *unique_ptr<string>($1);
        ast->func_r_params = nullptr;
        $$ = ast;
    }
    | IDENT '(' FuncRParams ')' {
        auto ast = new UnaryExpAST();
        ast->rule = 3;
        ast->ident = *unique_ptr<string>($1);
        ast->func_r_params = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

UnaryOp
    : '+' { auto op = new string("+"); $$ = op; }
    | '-' { auto op = new string("-"); $$ = op; }
    | '!' { auto op = new string("!"); $$ = op; }
    ;

FuncRParams
    : FuncRParams ',' Exp {
        auto ast = reinterpret_cast<FuncRParamsAST *>($1);
        ast->vec_func_r_params.push_back(unique_ptr<BaseAST>($3));
        $$ = ast;
    }
    | Exp {
        auto ast = new FuncRParamsAST();
        ast->vec_func_r_params.push_back(unique_ptr<BaseAST>($1));
        $$ = ast;
    }
    ;

MulExp
    : UnaryExp {
        auto ast = new MulExpAST();
        ast->rule = 1;
        ast->unaryexp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | MulExp '*' UnaryExp {
        auto ast = new MulExpAST();
        ast->rule = 2;
        ast->mulexp = unique_ptr<BaseAST>($1);
        ast->op = "*";
        ast->unaryexp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | MulExp '/' UnaryExp {
        auto ast = new MulExpAST();
        ast->rule = 3;
        ast->mulexp = unique_ptr<BaseAST>($1);
        ast->op = "/";
        ast->unaryexp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | MulExp '%' UnaryExp {
        auto ast = new MulExpAST();
        ast->rule = 4;
        ast->mulexp = unique_ptr<BaseAST>($1);
        ast->op = "%";
        ast->unaryexp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

AddExp
    : MulExp {
        auto ast = new AddExpAST();
        ast->rule = 1;
        ast->mulexp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | AddExp '+' MulExp {
        auto ast = new AddExpAST();
        ast->rule = 2;
        ast->addexp = unique_ptr<BaseAST>($1);
        ast->op = "+";
        ast->mulexp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | AddExp '-' MulExp {
        auto ast = new AddExpAST();
        ast->rule = 3;
        ast->addexp = unique_ptr<BaseAST>($1);
        ast->op = "-";
        ast->mulexp = unique_ptr<BaseAST>($3);
        $$ = ast;
    };

RelExp
    : AddExp {
        auto ast = new RelExpAST();
        ast->rule = 1;
        ast->addexp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | RelExp '<' AddExp {
        auto ast = new RelExpAST();
        ast->rule = 2;
        ast->relexp = unique_ptr<BaseAST>($1);
        ast->op = "<";
        ast->addexp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | RelExp '>' AddExp {
        auto ast = new RelExpAST();
        ast->rule = 3;
        ast->relexp = unique_ptr<BaseAST>($1);
        ast->op = ">";
        ast->addexp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | RelExp ORDEREDCOMPOP AddExp {
        auto ast = new RelExpAST();
        ast->rule = 4;
        ast->relexp = unique_ptr<BaseAST>($1);
        ast->op = *unique_ptr<string>($2);
        ast->addexp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

EqExp
    : RelExp {
        auto ast = new EqExpAST();
        ast->rule = 1;
        ast->relexp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | EqExp UNORDEREDCOMPOP RelExp {
        auto ast = new EqExpAST();
        ast->rule = 2;
        ast->eqexp = unique_ptr<BaseAST>($1);
        ast->op = *unique_ptr<string>($2);
        ast->relexp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

LAndExp
    : EqExp {
        auto ast = new LAndExpAST();
        ast->rule = 1;
        ast->id = l_and_exp_id++;
        ast->eqexp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | LAndExp LOGICAND EqExp {
        auto ast = new LAndExpAST();
        ast->rule = 2;
        ast->id = l_and_exp_id++;
        ast->landexp = unique_ptr<BaseAST>($1);
        ast->op = *unique_ptr<string>($2);
        ast->eqexp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

LOrExp
    : LAndExp {
        auto ast = new LOrExpAST();
        ast->rule = 1;
        ast->id = l_or_exp_id++;
        ast->landexp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | LOrExp LOGICOR LAndExp {
        auto ast = new LOrExpAST();
        ast->rule = 2;
        ast->id = l_or_exp_id++;
        ast->lorexp = unique_ptr<BaseAST>($1);
        ast->op = *unique_ptr<string>($2);
        ast->landexp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

ConstExp
    : Exp {
        auto ast = new ConstExpAST();
        ast->exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
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
