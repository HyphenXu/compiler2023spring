/* Options; for header file */
%code requires {
    #include <memory>
    #include <string>
    #include "ast.h"
}

%{
/* This part is for source code, instead of header file. */
#include <memory>
#include <string>
#include "ast.h"

#include <iostream>
#include <vector>

/* Necessary declarations. */
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

static int block_id = 1; /* Global block is ZERO */
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
    // TA's words:
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
%type <ast_val> StartSymbol

%type <ast_val> CompUnits CompUnit

%type <ast_val> Decl
%type <ast_val> ConstDecl ConstDefs ConstDef ConstInitVal
%type <ast_val> VarDecl VarDefs VarDef InitVal
%type <ast_val> ConstInitVals ConstExps InitVals Exps

%type <ast_val> FuncDef FuncFParams FuncFParam

%type <ast_val> Block BlockItems BlockItem GeneralStmt Stmt OpenStmt

%type <ast_val> Exp
%type <ast_val> LVal
%type <ast_val> PrimaryExp UnaryExp FuncRParams
%type <ast_val> MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <ast_val> ConstExp

%type <int_val> Number
%type <str_val> UnaryOp

%start StartSymbol

%%

/* Part 0: StartSymbol */
StartSymbol
    : CompUnits {
        auto start_symbol = make_unique<StartSymbolAST>();
        start_symbol->comp_units = unique_ptr<BaseAST>($1);
        ast = move(start_symbol);
    }
    ;

/* Part 1: CompUnit */
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

/** The basic semantics here is: after CompUnit is parsed, what shall be
 *  done with the return value of parsing FuncDef, and what shall be
 *  returned to the caller of this parser, with the pre-defined
 *  <%parse-param> "ast".
 *  $1 here is 'the return value of the first symbol'
 */
CompUnit
    : FuncDef {
        auto ast = new CompUnitAST();
        ast->type = COMP_UNIT_FUNC_DEF;
        ast->comp_unit = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | Decl {
        auto ast = new CompUnitAST();
        ast->type = COMP_UNIT_DECL;
        ast->comp_unit = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

/* Part 2: Decl */
Decl
    : ConstDecl {
        auto ast = new DeclAST();
        ast->type = DECL_CONST_DECL;
        ast->decl = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | VarDecl {
        auto ast = new DeclAST();
        ast->type = DECL_VAR_DECL;
        ast->decl = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

ConstDecl
    : CONST INT ConstDefs ';' {
        auto ast = new ConstDeclAST();
        ast->b_type = string("int");
        ast->const_defs = unique_ptr<BaseAST>($3);
        $$ = ast;
    };

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
        ast->const_exps = nullptr;
        ast->const_init_val = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | IDENT ConstExps '=' ConstInitVal {
        auto ast = new ConstDefAST();
        ast->ident = *unique_ptr<string>($1);
        ast->const_exps = unique_ptr<BaseAST>($2);
        ast->const_init_val = unique_ptr<BaseAST>($4);
        $$ = ast;
    }
    ;

ConstInitVal
    : ConstExp {
        auto ast = new ConstInitValAST();
        ast->type = 0;
        ast->const_exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | '{' ConstInitVals '}' {
        auto ast = new ConstInitValAST();
        ast->type = 1;
        ast->const_init_vals = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    | '{' '}' {
        auto ast = new ConstInitValAST();
        ast->type = 1;
        ast->const_init_vals = nullptr;
        $$ = ast;
    }
    ;

ConstInitVals
    : ConstInitVals ',' ConstInitVal {
        auto ast = reinterpret_cast<ConstInitValsAST *>($1);
        ast->vec_const_init_vals.push_back(unique_ptr<BaseAST>($3));
        $$ = ast;
    }
    | ConstInitVal {
        auto ast = new ConstInitValsAST();
        ast->vec_const_init_vals.push_back(unique_ptr<BaseAST>($1));
        $$ = ast;
    }
    ;

/* ConstInitValArray
    : ConstInitValArray ',' ConstExp {
        auto ast = reinterpret_cast<ConstInitValArrayAST *>($1);
        ast->vec_const_exps.push_back(unique_ptr<BaseAST>($3));
        $$ = ast;
    }
    | ConstExp {
        auto ast = new ConstInitValArrayAST();
        ast->vec_const_exps.push_back(unique_ptr<BaseAST>($1));
        $$ = ast;
    }
    ; */

ConstExps
    : ConstExps '[' ConstExp ']' {
        auto ast = reinterpret_cast<ConstExpsAST *>($1);
        ast->vec_const_exps.push_back(unique_ptr<BaseAST>($3));
        $$ = ast;
    }
    | '[' ConstExp ']' {
        auto ast = new ConstExpsAST();
        ast->vec_const_exps.push_back(unique_ptr<BaseAST>($2));
        $$ = ast;
    }
    ;

VarDecl
    : INT VarDefs ';' {
        auto ast = new VarDeclAST();
        ast->b_type = string("int");
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
        ast->const_exps = nullptr;
        ast->init_val = nullptr;
        $$ = ast;
    }
    | IDENT '=' InitVal {
        auto ast = new VarDefAST();
        ast->ident = *unique_ptr<string>($1);
        ast->const_exps = nullptr;
        ast->init_val = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | IDENT ConstExps {
        auto ast = new VarDefAST();
        ast->ident = *unique_ptr<string>($1);
        ast->const_exps = unique_ptr<BaseAST>($2);
        ast->init_val = nullptr;
        $$ = ast;
    }
    | IDENT ConstExps '=' InitVal {
        auto ast = new VarDefAST();
        ast->ident = *unique_ptr<string>($1);
        ast->const_exps = unique_ptr<BaseAST>($2);
        ast->init_val = unique_ptr<BaseAST>($4);
        $$ = ast;
    }
    ;

InitVal
    : Exp {
        auto ast = new InitValAST();
        ast->type = 0;
        ast->exp = unique_ptr<BaseAST>($1);
        ast->init_vals = nullptr;
        $$ = ast;
    }
    | '{' InitVals '}' {
        auto ast = new InitValAST();
        ast->type = 1;
        ast->exp = nullptr;
        ast->init_vals = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    | '{' '}' {
        auto ast = new InitValAST();
        ast->type = 1;
        ast->exp = nullptr;
        ast->init_vals = nullptr;
        $$ = ast;
    }
    ;

InitVals
    : InitVals ',' InitVal {
        auto ast = reinterpret_cast<InitValsAST *>($1);
        ast->vec_init_vals.push_back(unique_ptr<BaseAST>($3));
        $$ = ast;
    }
    | InitVal {
        auto ast = new InitValsAST();
        ast->vec_init_vals.push_back(unique_ptr<BaseAST>($1));
        $$ = ast;
    }
    ;

/* InitValArray
    : InitValArray ',' Exp {
        auto ast = reinterpret_cast<InitValArrayAST *>($1);
        ast->vec_exps.push_back(unique_ptr<BaseAST>($3));
        $$ = ast;
    }
    | Exp {
        auto ast = new InitValArrayAST();
        ast->vec_exps.push_back(unique_ptr<BaseAST>($1));
        $$ = ast;
    }
    ; */

/* Part 3: Func */
/** Miscs:
 *  Pay attention to the style here:
        - catch a pointer to a string with unique_ptr<string>
        - dereference and concatenate
        - pass the pointer to the newly constructed string
 *  $$ is the return value of the non-terminators.
 */
FuncDef
    : INT IDENT '(' ')' Block {
        auto ast = new FuncDefAST();
        ast->func_type = string("int");
        ast->ident = *unique_ptr<string>($2);
        ast->func_f_params = nullptr;
        ast->block = unique_ptr<BaseAST>($5);
        $$ = ast;
    }
    | INT IDENT '(' FuncFParams ')' Block {
        auto ast = new FuncDefAST();
        ast->func_type = string("int");
        ast->ident = *unique_ptr<string>($2);
        ast->func_f_params = unique_ptr<BaseAST>($4);
        ast->block = unique_ptr<BaseAST>($6);
        $$ = ast;
    }
    | VOID IDENT '(' ')' Block {
        auto ast = new FuncDefAST();
        ast->func_type = string("void");
        ast->ident = *unique_ptr<string>($2);
        ast->func_f_params = nullptr;
        ast->block = unique_ptr<BaseAST>($5);
        $$ = ast;
    }
    | VOID IDENT '(' FuncFParams ')' Block {
        auto ast = new FuncDefAST();
        ast->func_type = string("void");
        ast->ident = *unique_ptr<string>($2);
        ast->func_f_params = unique_ptr<BaseAST>($4);
        ast->block = unique_ptr<BaseAST>($6);
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
    : INT IDENT {
        auto ast = new FuncFParamAST();
        ast->b_type = string("int");
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
        ast->type = BLOCK_ITEM_DECL;
        ast->item = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | GeneralStmt {
        auto ast = new BlockItemAST();
        ast->type = BLOCK_ITEM_GENERAL_STMT;
        ast->item = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

GeneralStmt
    : Stmt {
        auto ast = new GeneralStmtAST();
        ast->type = GENERAL_STMT_STMT;
        ast->stmt = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | OpenStmt {
        auto ast = new GeneralStmtAST();
        ast->type = GENERAL_STMT_OPEN_STMT;
        ast->stmt = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

Stmt
    : LVal '=' Exp ';' {
        auto ast = new StmtAST();
        ast->type = STMT_ASSIGN;
        ast->l_val = unique_ptr<BaseAST>($1);
        ast->exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | Exp ';' {
        auto ast = new StmtAST();
        ast->type = STMT_EXP;
        ast->exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | ';' {
        auto ast = new StmtAST();
        ast->type = STMT_EXP;
        ast->exp = nullptr;
        $$ = ast;
    }
    | Block {
        auto ast = new StmtAST();
        ast->type = STMT_BLOCK;
        ast->block = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | RETURN Exp ';' {
        auto ast = new StmtAST();
        ast->type = STMT_RETURN;
        ast->ret_id = ret_id++;
        ast->exp = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    | RETURN ';' {
        auto ast = new StmtAST();
        ast->type = STMT_RETURN;
        ast->ret_id = ret_id++;
        ast->exp = nullptr;
        $$ = ast;
    }
    | IF '(' Exp ')' Stmt ELSE Stmt {
        auto ast = new StmtAST();
        ast->type = STMT_IF_STMT;
        ast->if_stmt_id = if_id++;
        ast->exp = unique_ptr<BaseAST>($3);
        ast->stmt_true = unique_ptr<BaseAST>($5);
        ast->stmt_false = unique_ptr<BaseAST>($7);
        $$ = ast;
    }
    | WHILE '(' Exp ')' Stmt {
        auto ast = new StmtAST();
        ast->type = STMT_WHILE_STMT;
        ast->while_id = while_id++;
        ast->exp = unique_ptr<BaseAST>($3);
        ast->stmt_body = unique_ptr<BaseAST>($5);
        $$ = ast;
    }
    | BREAK ';' {
        auto ast = new StmtAST();
        ast->type = STMT_BREAK;
        $$ = ast;
    }
    | CONTINUE ';' {
        auto ast = new StmtAST();
        ast->type = STMT_CONTINUE;
        $$ = ast;
    }
    ;

OpenStmt
    : IF '(' Exp ')' GeneralStmt {
        auto ast = new OpenStmtAST();
        ast->type = OPEN_STMT_IF_GENERAL_STMT;
        ast->if_stmt_id = if_id++;
        ast->exp = unique_ptr<BaseAST>($3);
        ast->stmt_true = unique_ptr<BaseAST>($5);
        $$ = ast;
    }
    | IF '(' Exp ')' Stmt ELSE OpenStmt {
        auto ast = new OpenStmtAST();
        ast->type = OPEN_STMT_IF_STMT_OPEN_STMT;
        ast->if_stmt_id = if_id++;
        ast->exp = unique_ptr<BaseAST>($3);
        ast->stmt_true = unique_ptr<BaseAST>($5);
        ast->stmt_false = unique_ptr<BaseAST>($7);
        $$ = ast;
    }
    | WHILE '(' Exp ')' OpenStmt {
        auto ast = new OpenStmtAST();
        ast->type = OPEN_STMT_WHILE_OPEN_STMT;
        ast->while_id = while_id++;
        ast->exp = unique_ptr<BaseAST>($3);
        ast->stmt_body = unique_ptr<BaseAST>($5);
        $$ = ast;
    }
    ;

/* Part 5: Exp */
Exp
    : LOrExp {
        auto ast = new ExpAST();
        ast->l_or_exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    ;

LVal
    : IDENT {
        auto ast = new LValAST();
        ast->ident = *unique_ptr<string>($1);
        ast->exps = nullptr;
        $$ = ast;
    }
    | IDENT Exps {
        auto ast = new LValAST();
        ast->ident = *unique_ptr<string>($1);
        ast->exps = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    ;

Exps
    : Exps '[' Exp ']' {
        auto ast = reinterpret_cast<ExpsAST *>($1);
        ast->vec_exps.push_back(unique_ptr<BaseAST>($3));
        $$ = ast;
    }
    | '[' Exp ']' {
        auto ast = new ExpsAST();
        ast->vec_exps.push_back(unique_ptr<BaseAST>($2));
        $$ = ast;
    }
    ;

PrimaryExp
    : '(' Exp ')' {
        auto ast = new PrimaryExpAST();
        ast->type = PRIMARY_EXP_EXP;
        ast->exp = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    | LVal {
        auto ast = new PrimaryExpAST();
        ast->type = PRIMARY_EXP_L_VAL;
        ast->l_val = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | Number {
        auto ast = new PrimaryExpAST();
        ast->type = PRIMARY_EXP_NUMBER;
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
        ast->type = UNARY_EXP_PRIMARY_EXP;
        ast->primary_exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | UnaryOp UnaryExp {
        auto ast = new UnaryExpAST();
        ast->type = UNARY_EXP_UNARY_OP_EXP;
        ast->unary_op = *unique_ptr<string>($1);
        ast->unary_exp = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    | IDENT '(' ')' {
        auto ast = new UnaryExpAST();
        ast->type = UNARY_EXP_FUNCTION_CALL;
        ast->ident = *unique_ptr<string>($1);
        ast->func_r_params = nullptr;
        $$ = ast;
    }
    | IDENT '(' FuncRParams ')' {
        auto ast = new UnaryExpAST();
        ast->type = UNARY_EXP_FUNCTION_CALL;
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
        ast->type = MUL_EXP_UNARY;
        ast->unary_exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | MulExp '*' UnaryExp {
        auto ast = new MulExpAST();
        ast->type = MUL_EXP_MUL;
        ast->mul_exp = unique_ptr<BaseAST>($1);
        ast->op = "*";
        ast->unary_exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | MulExp '/' UnaryExp {
        auto ast = new MulExpAST();
        ast->type = MUL_EXP_DIV;
        ast->mul_exp = unique_ptr<BaseAST>($1);
        ast->op = "/";
        ast->unary_exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | MulExp '%' UnaryExp {
        auto ast = new MulExpAST();
        ast->type = MUL_EXP_MOD;
        ast->mul_exp = unique_ptr<BaseAST>($1);
        ast->op = "%";
        ast->unary_exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

AddExp
    : MulExp {
        auto ast = new AddExpAST();
        ast->type = ADD_EXP_MUL;
        ast->mul_exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | AddExp '+' MulExp {
        auto ast = new AddExpAST();
        ast->type = ADD_EXP_ADD;
        ast->add_exp = unique_ptr<BaseAST>($1);
        ast->op = "+";
        ast->mul_exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | AddExp '-' MulExp {
        auto ast = new AddExpAST();
        ast->type = ADD_EXP_SUB;
        ast->add_exp = unique_ptr<BaseAST>($1);
        ast->op = "-";
        ast->mul_exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    };

RelExp
    : AddExp {
        auto ast = new RelExpAST();
        ast->type = REL_EXP_ADD;
        ast->add_exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | RelExp '<' AddExp {
        auto ast = new RelExpAST();
        ast->type = REL_EXP_LT;
        ast->rel_exp = unique_ptr<BaseAST>($1);
        ast->op = "<";
        ast->add_exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | RelExp '>' AddExp {
        auto ast = new RelExpAST();
        ast->type = REL_EXP_GT;
        ast->rel_exp = unique_ptr<BaseAST>($1);
        ast->op = ">";
        ast->add_exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    | RelExp ORDEREDCOMPOP AddExp {
        auto ast = new RelExpAST();
        ast->type = REL_EXP_ORDERED;
        ast->rel_exp = unique_ptr<BaseAST>($1);
        ast->op = *unique_ptr<string>($2);
        ast->add_exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

EqExp
    : RelExp {
        auto ast = new EqExpAST();
        ast->type = EQ_EXP_REL;
        ast->rel_exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | EqExp UNORDEREDCOMPOP RelExp {
        auto ast = new EqExpAST();
        ast->type = EQ_EXP_UNORDERED;
        ast->eq_exp = unique_ptr<BaseAST>($1);
        ast->op = *unique_ptr<string>($2);
        ast->rel_exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

LAndExp
    : EqExp {
        auto ast = new LAndExpAST();
        ast->type = L_AND_EXP_EQ;
        ast->id = l_and_exp_id++;
        ast->eq_exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | LAndExp LOGICAND EqExp {
        auto ast = new LAndExpAST();
        ast->type = L_AND_EXP_L_AND;
        ast->id = l_and_exp_id++;
        ast->l_and_exp = unique_ptr<BaseAST>($1);
        ast->op = *unique_ptr<string>($2);
        ast->eq_exp = unique_ptr<BaseAST>($3);
        $$ = ast;
    }
    ;

LOrExp
    : LAndExp {
        auto ast = new LOrExpAST();
        ast->type = L_OR_EXP_L_AND;
        ast->id = l_or_exp_id++;
        ast->l_and_exp = unique_ptr<BaseAST>($1);
        $$ = ast;
    }
    | LOrExp LOGICOR LAndExp {
        auto ast = new LOrExpAST();
        ast->type = L_OR_EXP_L_OR;
        ast->id = l_or_exp_id++;
        ast->l_or_exp = unique_ptr<BaseAST>($1);
        ast->op = *unique_ptr<string>($2);
        ast->l_and_exp = unique_ptr<BaseAST>($3);
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
