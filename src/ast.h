#ifndef AST_H
#define AST_H

#include <memory>
#include <iostream>
#include <sstream>

#include <cassert>

class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;

    virtual void Dump2String(std::stringstream &ss) const = 0;
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

    void Dump2String(std::stringstream &ss) const override {
        ss << "ret ";
        ss << number;
        ss << std::endl;
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

    void Dump2String(std::stringstream &ss) const override {
        ss << "%entry:" << std::endl;
            ss << "\t";
            stmt->Dump2String(ss);
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

    void Dump2String(std::stringstream &ss) const override {
        ss << "i32";
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

    void Dump2String(std::stringstream &ss) const override {
        ss << "fun ";
        ss << "@main(";
        ss << "): ";
        func_type->Dump2String(ss);
        ss << " {" << std::endl;
        block->Dump2String(ss);
        ss << "}" << std::endl;
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

    void Dump2String(std::stringstream &ss) const override {
        func_def->Dump2String(ss);
    }
};

#endif /**< src/ast.h */