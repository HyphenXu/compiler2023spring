#ifndef AST_H
#define AST_H

#include <memory>
#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <stack>
#include <string>

#include "symbol.h"

static int result_id = 0;
static std::map<std::string, SymbolTable> map_func2symbolTable;
static std::stack<std::string> stack_namespace;

typedef struct{
    int depth;
    int result_number;
    int result_id;
} exp_result_t;

typedef struct{
    bool lhs;
    int val;
} l_val_result_t;

/* Base AST class */
class BaseAST;

/* Part 1: CompUnit */
class CompUnitAST;

/* Part 2: Decl */
class DeclAST;
class ConstDeclAST;
class BtypeAST;
class ConstDefsAST;
class ConstDefAST;
class ConstInitValAST;

/* Part 3: Func */
class FuncDefAST;
class FuncTypeAST;

/* Part 4: Block */
class BlockAST;
class BlockItemsAST;
class BlockItemAST;
class StmtAST;

/* Part 5: Exp */
class ExpAST;
class LValAST;
class PrimaryExpAST;
class UnaryExpAST;
class MulExpAST;
class AddExpAST;
class RelExpAST;
class EqExpAST;
class LAndExpAST;
class LOrExpAST;
class ConstExpAST;

/* Base AST class */
class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;

    virtual void Dump2StringIR(void *aux) const = 0;
};

/* Part 1: CompUnit */
/* CompUnit  ::= FuncDef; */
class CompUnitAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override {
        std::cout << "CompUnitAST { ";
        func_def->Dump();
        std::cout << " }";
        std::cout << std::endl;
    }

    void Dump2StringIR(void *aux) const override {
        func_def->Dump2StringIR(nullptr);
    }
};

/* Part 2: Decl */
/* Decl          ::= ConstDecl; */
class DeclAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> const_decl;

    void Dump() const override {
        std::cout << "DeclAST { ";
        const_decl->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        const_decl->Dump2StringIR(nullptr);
    }
};

/* ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";"; */
class ConstDeclAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> b_type;
    std::unique_ptr<BaseAST> const_defs;

    void Dump() const override {
        std::cout << "ConstDeclAST { ";
        b_type->Dump();
        const_defs->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        std::string string_b_type;
        b_type->Dump2StringIR(&string_b_type);
        /* TODO: what if other type? */
        assert(string_b_type == "int");

        const_defs->Dump2StringIR(&string_b_type);
    }
};

/* BType         ::= "int"; */
class BTypeAST : public BaseAST {
public:
    std::string type_string;

    void Dump() const override {
        std::cout << "BTypeAST { " << type_string << " }";
    }

    void Dump2StringIR(void *aux) const override {
        std::string *string_b_type = (std::string *)aux;
        (*string_b_type) = type_string;
    }
};


class ConstDefsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_const_defs;

    void Dump() const override {
        std::cout << "ConstDefsAST { ";
        // std::vector<std::unique_ptr<BaseAST> >::iterator ii, ie;
        auto ii = vec_const_defs.begin();
        auto ie = vec_const_defs.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        auto ii = vec_const_defs.begin();
        auto ie = vec_const_defs.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump2StringIR(aux);
        }
    }
};

/* ConstDef      ::= IDENT "=" ConstInitVal; */
class ConstDefAST : public BaseAST {
public:
    std::string ident;
    std::unique_ptr<BaseAST> const_init_val;

    void Dump() const override {
        std::cout << "ConstDefAST { ";
        std::cout << ident << " = ";
        const_init_val->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        std::string *string_b_type = (std::string *)aux;
        /* TODO: what if other type */
        assert((*string_b_type) == "int");

        std::string cur_namespace = stack_namespace.top();
        assert(!map_func2symbolTable[cur_namespace].bool_exist(ident));

        exp_result_t const_exp_result;
        const_init_val->Dump2StringIR(&const_exp_result);
        assert(const_exp_result.depth == 0);
        int val = const_exp_result.result_number;

        map_func2symbolTable[cur_namespace].insert_definition(ident, val);
        /* TODO: insert type */
    }
};

/* ConstInitVal  ::= ConstExp; */
class ConstInitValAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> const_exp;

    void Dump() const override {
        std::cout << "ConstInitValAST { ";
        const_exp->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        const_exp->Dump2StringIR(aux);
    }
};

/* Part 3: Func */
/* FuncDef   ::= FuncType IDENT "(" ")" Block; */
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override {
        std::cout << "FuncDefAST { ";
        func_type->Dump();
        std::cout << ", " << ident << ", ";
        block->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        map_func2symbolTable[ident] = SymbolTable();
        stack_namespace.push(ident);

        std::cout << "fun ";
        std::cout << "@" << ident << "(";
        std::cout << "): ";
        /* TODO: type check? */
        std::string string_type;
        func_type->Dump2StringIR(&string_type);
        std::cout << " {" << std::endl;
        block->Dump2StringIR(nullptr);
        std::cout << "}" << std::endl;

        stack_namespace.pop();
        map_func2symbolTable[ident].clear_table();
    }
};

/* FuncType  ::= "int"; */
class FuncTypeAST : public BaseAST {
public:
    std::string type_string;

    void Dump() const override {
        std::cout << "FuncTypeAST { ";
        std::cout << type_string;
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        assert(type_string == "int");
        *((std::string *)aux) = type_string;
        std::cout << "i32";
    }
};

/* Part 4: Block */
/* Block         ::= "{" {BlockItem} "}"; */
class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> block_items;

    void Dump() const override {
        std::cout << "BlockAST { ";
        if(block_items != nullptr){
            block_items->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        std::cout << "%" << "entry" << ":" << std::endl;
        if(block_items != nullptr){
            block_items->Dump2StringIR(nullptr);
        }
        else{
            std::cout << "\tret 0" << std::endl;
        }
    }
};

class BlockItemsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_block_items;

    void Dump() const override {
        std::cout << "BlockItemsAST { ";
        // std::vector<std::unique_ptr<BaseAST> >::iterator ii, ie;
        auto ii = vec_block_items.begin();
        auto ie = vec_block_items.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        auto ii = vec_block_items.begin();
        auto ie = vec_block_items.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump2StringIR(nullptr);
        }
    }
};

/* BlockItem     ::= Decl | Stmt; */
class BlockItemAST : public BaseAST {
public:
    bool is_stmt;
    std::unique_ptr<BaseAST> item;

    void Dump() const override {
        std::cout << "BlockItemAST { ";
        std::cout << "is_stmt: " << is_stmt << ", ";
        item->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        item->Dump2StringIR(nullptr);
    }
};

/* Stmt      ::= "return" Exp ";"; */
class StmtAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << "StmtAST { ";
        std::cout << "return ";
        exp->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        exp_result_t exp_result;
        exp->Dump2StringIR(&exp_result);

        if(exp_result.depth == 0){
            std::cout << "\tret " << exp_result.result_number << std::endl;
        }
        else{
            std::cout << "\tret %" << exp_result.result_id << std::endl;
        }
    }
};

/* Part 5: Exp */
/* Exp         ::= LOrExp; */
class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> lorexp;

    void Dump() const override {
        std::cout << "ExpAST { ";
        lorexp->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        lorexp->Dump2StringIR(aux);
    }
};

/* LVal          ::= IDENT; */
class LValAST : public BaseAST {
public:
    std::string ident;

    void Dump() const override {
        std::cout << "LValAST { " << ident << " }";
    }

    void Dump2StringIR(void *aux) const override {
        std::string cur_namespace = stack_namespace.top();

        if(((l_val_result_t *)aux)->lhs){
            /* TODO */
        }
        else{
            ((l_val_result_t *)aux)->val =
                map_func2symbolTable[cur_namespace].get_definition[ident];
        }
    }
};

/*
    PrimaryExp    ::= "(" Exp ")" | LVal | Number;
    Number      ::= INT_CONST;
*/
class PrimaryExpAST : public BaseAST {
public:
    int rule;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> l_val;
    int number;

    void Dump() const override {
        std::cout << "PrimaryExpAST { ";
        if(rule == 1){
            std::cout << "( ";
            exp->Dump();
            std::cout << " )";
        }
        else if(rule == 2){
            std::cout << number;
        }
        else if(rule == 3){
            l_val->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        if(rule == 1){
            exp->Dump2StringIR(aux);
        }
        else if(rule == 2){
            ((exp_result_t *)aux)->depth = 0;
            ((exp_result_t *)aux)->result_number = number;
        }
        else if(rule == 3){
            l_val_result_t l_val_result;
            l_val_result.lhs = false;
            l_val->Dump2StringIR(&l_val_result);

            ((exp_result_t *)aux)->depth = 0;
            ((exp_result_t *)aux)->result_number = l_val_result.val;
        }
    }
};

/*
    UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
    UnaryOp     ::= "+" | "-" | "!";
*/
class UnaryExpAST : public BaseAST {
public:
    int rule;
    std::unique_ptr<BaseAST> primaryexp;
    std::string unaryop;
    std::unique_ptr<BaseAST> unaryexp;

    void Dump() const override {
        std::cout << "UnaryExpAST { ";
        if(rule == 1){
            primaryexp->Dump();
        }
        else if(rule == 2){
            std::cout << unaryop;
            unaryexp->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        if(rule == 1){
            primaryexp->Dump2StringIR(aux);
        }
        else if(rule == 2){
            assert(unaryop == "+" || unaryop == "-" || unaryop == "!");
            exp_result_t result1;
            exp_result_t *result = (exp_result_t *)aux;

            unaryexp->Dump2StringIR(&result1);

            if(result1.depth == 0){
                result->depth = 0;
                switch (unaryop[0])
                {
                case '+':
                    result->result_number = result1.result_number;
                    break;
                case '-':
                    result->result_number = -result1.result_number;
                    break;
                case '!':
                    result->result_number = !result1.result_number;
                    break;
                default:
                    assert(false);
                    break;
                }
            }
            else{
                switch (unaryop[0])
                {
                case '+':
                    /* do nothing */
                    result->depth = result1.depth;
                    result->result_id = result_id;
                    break;
                case '-':
                    result->depth = result1.depth + 1;
                    result->result_id = result_id++;
                    std::cout << "\t%" << result->result_id << " = sub 0, ";
                    std::cout << "%" << result1.result_id << std::endl;
                    break;
                case '!':
                    result->depth = result1.depth + 1;
                    result->result_id = result_id++;
                    std::cout << "\t%" << result->result_id << " = eq 0, ";
                    std::cout << "%" << result1.result_id << std::endl;
                    break;
                default:
                    assert(false);
                    break;
                }
            }
        }
    }
};

/* MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp; */
class MulExpAST : public BaseAST {
public:
    int rule;
    std::unique_ptr<BaseAST> unaryexp;
    std::unique_ptr<BaseAST> mulexp;
    std::string op;

    void Dump() const override {
        std::cout << "MulExpAST { ";
        if(rule == 1){
            unaryexp->Dump();
        }
        else{
            mulexp->Dump();
            std::cout << " " << op << " ";
            unaryexp->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        if(rule == 1){
            unaryexp->Dump2StringIR(aux);
        }
        else{
            assert(op == "%" || op == "*" || op == "/");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            mulexp->Dump2StringIR(&result1);
            unaryexp->Dump2StringIR(&result2);

            if(result1.depth == 0 && result2.depth == 0){
                result->depth = 0;
                switch (op[0])
                {
                case '/':
                    result->result_number = result1.result_number
                                            / result2.result_number;
                    break;
                case '*':
                    result->result_number = result1.result_number
                                            * result2.result_number;
                    break;
                case '%':
                    result->result_number = result1.result_number
                                            % result2.result_number;
                    break;
                default:
                    assert(false);
                    break;
                }
            }
            else{
                result->depth = std::max(result1.depth, result2.depth) + 1;
                result->result_id = result_id++;

                switch (op[0])
                {
                case '/':
                    std::cout << "\t%" << ret.result_id << " = div ";
                    break;
                case '*':
                    std::cout << "\t%" << ret.result_id << " = mul ";
                    break;
                case '%':
                    std::cout << "\t%" << ret.result_id << " = mod ";
                    break;
                default:
                    assert(false);
                    break;
                }

                if(result1.depth == 0){
                    std::cout << result1.result_number;
                }
                else{
                    std::cout << "%" << result1.result_id;
                }
                std::cout << ", ";
                if(result2.depth == 0){
                    std::cout << result2.result_number;
                }
                else{
                    std::cout << "%" << result2.result_id;
                }
                std::cout << std::endl;
            }
        }
    }
};

/* AddExp      ::= MulExp | AddExp ("+" | "-") MulExp; */
class AddExpAST : public BaseAST {
public:
    int rule;
    std::unique_ptr<BaseAST> mulexp;
    std::unique_ptr<BaseAST> addexp;
    std::string op;

    void Dump() const override {
        std::cout << "AddExpAST { ";
        if(rule == 1){
            mulexp->Dump();
        }
        else{
            addexp->Dump();
            std::cout << " " << op << " ";
            mulexp->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        if(rule == 1){
            mulexp->Dump2StringIR(aux);
        }
        else{
            assert(op == "+" || op == "-");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            addexp->Dump2StringIR(&result1);
            mulexp->Dump2StringIR(&result2);

            if(result1.depth == 0 && result2.depth == 0){
                result->depth = 0;
                switch (op[0])
                {
                case '+':
                    result->result_number = result1.result_number +
                                            result2.result_number;
                    break;
                case '-':
                    result->result_number = result1.result_number -
                                            result2.result_number;
                    break;
                default:
                    assert(false);
                    break;
                }
            }
            else{
                result->depth = std::max(result1.depth, result2.depth) + 1;
                result->result_id = result_id++;

                switch (op[0])
                {
                case '+':
                    std::cout << "\t%" << result->result_id << " = add ";
                    break;
                case '-':
                    std::cout << "\t%" << result->result_id << " = sub ";
                    break;
                default:
                    assert(false);
                    break;
                }

                if(result1.depth == 0){
                    std::cout << result1.result_number;
                }
                else{
                    std::cout << "%" << result1.result_id;
                }
                std::cout << ", ";
                if(result2.depth == 0){
                    std::cout << result2.result_number;
                }
                else{
                    std::cout << "%" << result2.result_id;
                }
                std::cout << std::endl;
            }
        }
    }
};

/* RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp; */
class RelExpAST : public BaseAST {
public:
    int rule;
    std::unique_ptr<BaseAST> addexp;
    std::unique_ptr<BaseAST> relexp;
    std::string op;

    void Dump() const override {
        std::cout << "RelExpAST { ";
        if(rule == 1){
            addexp->Dump();
        }
        else{
            relexp->Dump();
            std::cout << " " << op << " ";
            addexp->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        if(rule == 1){
            addexp->Dump2StringIR(aux);
        }
        else{
            assert(op == "<" || op == ">" || op == "<=" || op == ">=");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            relexp->Dump2StringIR(&result1);
            addexp->Dump2StringIR(&result2);

            if(result1.depth == 0 && result2.depth == 0){
                result->depth = 0;
                if(op == "<"){
                    result->result_number = result1.result_number
                                            < result2.result_number;
                }
                else if(op == ">"){
                    result->result_number = result1.result_number
                                            > result2.result_number;
                }
                else if(op == "<="){
                    result->result_number = result1.result_number
                                            <= result2.result_number;
                }
                else if(op == ">="){
                    result->result_number = result1.result_number
                                            >= result2.result_number;
                }
            }
            else{
                result->depth = std::max(result1.depth, result2.depth) + 1;
                result->result_id = result_id++;

                if(op == "<"){
                    std::cout << "\t%" << result->result_id << " = lt ";
                }
                else if(op == ">"){
                    std::cout << "\t%" << result->result_id << " = gt ";
                }
                else if(op == "<="){
                    std::cout << "\t%" << result->result_id << " = le ";
                }
                else if(op == ">="){
                    std::cout << "\t%" << result->result_id << " = ge ";
                }

                if(result1.depth == 0){
                    std::cout << result1.result_number;
                }
                else{
                    std::cout << "%" << result1.result_id;
                }
                std::cout << ", ";
                if(result2.depth == 0){
                    std::cout << result2.result_number;
                }
                else{
                    std::cout << "%" << result2.result_id;
                }
                std::cout << std::endl;
            }
        }
    }
};

/* EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp; */
class EqExpAST : public BaseAST {
public:
    int rule;
    std::unique_ptr<BaseAST> relexp;
    std::unique_ptr<BaseAST> eqexp;
    std::string op;

    void Dump() const override {
        std::cout << "EqExpAST { ";
        if(rule == 1){
            relexp->Dump();
        }
        else{
            eqexp->Dump();
            std::cout << " " << op << " ";
            relexp->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        if(rule == 1){
            relexp->Dump2StringIR(aux);
        }
        else{
            assert(op == "==" || op == "!=");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            relexp->Dump2StringIR(&result1);
            eqexp->Dump2StringIR(&result2);

            if(result1.depth == 0 && result2.depth == 0){
                result->depth = 0;
                if(op == "=="){
                    result->result_number = result1.result_number
                                        == result2.result_number;
                }
                else if(op == "!="){
                    result->result_number = result1.result_number
                                        != result2.result_number;
                }
            }
            else{
                result->depth = std::max(result1.depth, result2.depth) + 1;
                result->result_id = result_id++;

                if(op == "=="){
                    std::cout << "\t%" << result->result_id << " = eq ";
                }
                else if(op == "!="){
                    std::cout << "\t%" << result->result_id << " = ne ";
                }

                if(result1.depth == 0){
                    std::cout << result1.result_number;
                }
                else{
                    std::cout << "%" << result1.result_id;
                }
                std::cout << ", ";
                if(result2.depth == 0){
                    std::cout << result2.result_number;
                }
                else{
                    std::cout << "%" << result2.result_id;
                }
                std::cout << std::endl;
            }
        }
    }
};

/* LAndExp     ::= EqExp | LAndExp "&&" EqExp; */
class LAndExpAST : public BaseAST {
public:
    int rule;
    std::unique_ptr<BaseAST> eqexp;
    std::unique_ptr<BaseAST> landexp;
    std::string op;

    void Dump() const override {
        std::cout << "LAndExpAST { ";
        if(rule == 1){
            eqexp->Dump();
        }
        else{
            landexp->Dump();
            std::cout << " " << op << " ";
            eqexp->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        if(rule == 1){
            eqexp->Dump2StringIR(aux);
        }
        else{
            assert(op == "&&");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            landexp->Dump2StringIR(&result1);
            /* TODO: short-curcuit logic */
            eqexp->Dump2StringIR(&result2);

            if(result1.depth == 0 && result2.depth == 0){
                result->depth = 0;
                result->result_number = result1.result_number
                                        && result2.result_number;
            }
            else{
                std::cout << "\t%" << result_id++ << " = ne 0, ";
                if(result1.depth == 0){
                    std::cout << result1.result_number;
                }
                else{
                    std::cout << "%" << result1.result_id;
                }
                std::cout << std::endl;

                std::cout << "\t%" << result_id++ << " = ne 0, ";
                if(result2.depth == 0){
                    std::cout << result2.result_number;
                }
                else{
                    std::cout << "%" << result2.result_id;
                }
                std::cout << std::endl;

                result->depth = std::max(result1.depth, result2.depth) + 1;
                result->result_id = result_id++;

                std::cout   << "\t%" << result->result_id << " = and %"
                            << result->result_id - 1 << ", %"
                            << result->result_id - 2
                            << std::endl;
            }
        }
    }
};

/* LOrExp      ::= LAndExp | LOrExp "||" LAndExp; */
class LOrExpAST : public BaseAST {
public:
    int rule;
    std::unique_ptr<BaseAST> landexp;
    std::unique_ptr<BaseAST> lorexp;
    std::string op;

    void Dump() const override {
        std::cout << "LOrExpAST { ";
        if(rule == 1){
            landexp->Dump();
        }
        else{
            lorexp->Dump();
            std::cout << " " << op << " ";
            landexp->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        if(rule == 1){
            landexp->Dump2StringIR(aux);
        }
        else{
            assert(op == "||");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            lorexp->Dump2StringIR(&result1);
            /* TODO: short-circuit logic */
            landexp->Dump2StringIR(&result2);

            if(result1.depth == 0 && result2.depth == 0){
                result->depth = 0;
                result->result_number = result1.result_number
                                        || result2.result_number;
            }
            else{
                result->depth = std::max(result1.depth, result2.depth) + 1;
                result->result_id = result_id++;

                std::cout << "\t%" << result->result_id << " = or ";
                if(result1.depth == 0){
                    std::cout << result1.result_number;
                }
                else{
                    std::cout << "%" << result1.result_id;
                }
                std::cout << ", ";
                if(result2.depth == 0){
                    std::cout << result2.result_number;
                }
                else{
                    std::cout << "%" << result2.result_id;
                }
                std::cout << std::endl;

                result->result_id = result_id++;
                std::cout << "\t%" << result->result_id << " = ne 0, %";
                std::cout << result->result_id - 1 << std::endl;
            }
        }
    }
};

/* ConstExp      ::= Exp; */
class ConstExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << "ConstExpAST {";
        exp->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        exp->Dump2StringIR(aux);
    }
};

#endif /**< src/ast.h */
