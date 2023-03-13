#ifndef AST_H
#define AST_H

#include <memory>
#include <iostream>

#include <cassert>

class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;
};

/* 
    Stmt      ::= "return" Number ";";
    Number    ::= INT_CONST;
*/
class StmtAST : public BaseAST {
public:
    int number;

    void Dump() const override {
        std::cout << "StmtAST { ";
        std::cout << number;
        std::cout << " }";
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
};

/* CompUnit  ::= FuncDef; */
class CompUnitAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override {
        std::cout << "CompUnitAST { ";
        func_def->Dump();
        std::cout << " }";
    }
};

#endif /**< src/ast.h */