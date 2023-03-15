#ifndef AST_H
#define AST_H

#include <memory>
#include <iostream>
#include <sstream>

#include <cassert>

static int var_id = 0;

typedef struct{
    int depth;
    int number;
}ast_ret_t;

class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;

    virtual ast_ret_t Dump2String(std::stringstream &ss) const = 0;
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

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        ast_ret_t ret;
        if(rule == 1){
            ret = primaryexp->Dump2String(ss);
        }
        else if(rule == 2){
            ret = unaryexp->Dump2String(ss);
            if(unaryop == "+"){
                /* do nothing */
            }
            else if(unaryop == "-"){
                ss << "\t%" << var_id << " = sub 0, ";
                if(ret.depth == 0){
                    ss << ret.number << std::endl;
                }
                else{
                    ss << "%" << var_id - 1 << std::endl;
                }
                var_id++;
                ret.depth++;
            }
            else if(unaryop == "!"){
                ss << "\t%" << var_id << " = eq 0, ";
                if(ret.depth == 0){
                    ss << ret.number << std::endl;
                }
                else{
                    ss << "%" << var_id - 1 << std::endl;
                }
                var_id++;
                ret.depth++;
            }
            else{
                assert(false);
            }
        }
        return ret;
    }
};

/*
    PrimaryExp  ::= "(" Exp ")" | Number;
    Number      ::= INT_CONST;
*/
class PrimaryExpAST : public BaseAST {
public:
    int rule;
    std::unique_ptr<BaseAST> exp;
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
        std::cout << " }";
    }

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        ast_ret_t ret;
        if(rule == 1){
            ret = exp->Dump2String(ss);
        }
        else if(rule == 2){
            // ss << number;
            ret.number = number;
            ret.depth = 0;
        }
        return ret;
    }
};

/* Exp         ::= UnaryExp; */
class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> unaryexp;

    void Dump() const override {
        std::cout << "ExpAST { ";
        unaryexp->Dump();
        std::cout << " }";
    }

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        return unaryexp->Dump2String(ss);
    }
};

/* Stmt      ::= "return" Exp ";"; */
class StmtAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << "StmtAST { ";
        exp->Dump();
        std::cout << " }";
    }

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        ast_ret_t ret = exp->Dump2String(ss);

        if(ret.depth == 0){
            ss << "\tret " << ret.number << std::endl;
        }
        else{
            ss << "\tret %" << ret.depth - 1 << std::endl;
        }
        return ret;
    }
};

/* Block     ::= "{" Stmt "}"; */
class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override {
        std::cout << "BlockAST { ";
        stmt->Dump();
        std::cout << " }";
    }

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        ast_ret_t ret;
        ss << "%entry:" << std::endl;
        ret = stmt->Dump2String(ss);
        return ret;
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

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        ss << "i32";
        ast_ret_t ret;
        return ret;
    }
};

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

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        ss << "fun ";
        ss << "@main(";
        ss << "): ";
        func_type->Dump2String(ss);
        ss << " {" << std::endl;
        ast_ret_t ret = block->Dump2String(ss);
        ss << "}" << std::endl;
        return ret;
    }
};

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

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        return func_def->Dump2String(ss);
    }
};

#endif /**< src/ast.h */