#ifndef AST_H
#define AST_H

#include <memory>
#include <iostream>
#include <sstream>

#include <cassert>

static int result_id = 0;

typedef struct{
    int depth;
    int result_number;
    int result_id;
}ast_ret_t;

class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;

    virtual ast_ret_t Dump2String(std::stringstream &ss) const = 0;
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

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        if(rule == 1){
            return landexp->Dump2String(ss);
        }
        else{
            ast_ret_t ret;
            ast_ret_t ret1 = lorexp->Dump2String(ss);
            ast_ret_t ret2 = landexp->Dump2String(ss);
            ret.result_id = result_id++;
            assert(op == "||");
            ss << "\t%" << ret.result_id << " = or ";
            ret.result_number = ret1.result_number || ret2.result_number;

            ret.depth = std::max(ret1.depth, ret2.depth) + 1;
            if(ret1.depth == 0){
                ss << ret1.result_number;
            }
            else{
                ss << "%" << ret1.result_id;
            }
            ss << ", ";
            if(ret2.depth == 0){
                ss << ret2.result_number;
            }
            else{
                ss << "%" << ret2.result_id;
            }
            ss << std::endl;

            ret.result_id = result_id++;
            ss << "\t%" << ret.result_id << " = ne 0, %";
            ss << ret.result_id - 1 << std::endl;

            return ret;
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

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        if(rule == 1){
            return eqexp->Dump2String(ss);
        }
        else{
            ast_ret_t ret;
            ast_ret_t ret1 = landexp->Dump2String(ss);
            ast_ret_t ret2 = eqexp->Dump2String(ss);
            assert(op == "&&");

            ss << "\t%" << result_id++ << " = ne 0, ";
            if(ret1.depth == 0){
                ss << ret1.result_number;
            }
            else{
                ss << "%" << ret1.result_id;
            }
            ss << std::endl;

            ss << "\t%" << result_id++ << " = ne 0, ";
            if(ret2.depth == 0){
                ss << ret2.result_number;
            }
            else{
                ss << "%" << ret2.result_id;
            }
            ss << std::endl;

            ret.result_id = result_id++;
            ret.result_number = ret1.result_number && ret2.result_number;
            ret.depth = std::max(ret1.depth, ret2.depth) + 1;
            ss  << "\t%" << ret.result_id << " = and %"
                << ret.result_id - 1 << ", %" << ret.result_id - 2
                << std::endl;
            return ret;
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

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        if(rule == 1){
            return relexp->Dump2String(ss);
        }
        else{
            ast_ret_t ret;
            ast_ret_t ret1 = relexp->Dump2String(ss);
            ast_ret_t ret2 = eqexp->Dump2String(ss);
            ret.result_id = result_id++;
            if(op == "=="){
                ss << "\t%" << ret.result_id << " = eq ";
                ret.result_number = ret1.result_number == ret2.result_number;
            }
            else if(op == "!="){
                ss << "\t%" << ret.result_id << " = ne ";
                ret.result_number = ret1.result_number != ret2.result_number;
            }
            else{
                assert(false);
            }

            ret.depth = std::max(ret1.depth, ret2.depth) + 1;
            if(ret1.depth == 0){
                ss << ret1.result_number;
            }
            else{
                ss << "%" << ret1.result_id;
            }
            ss << ", ";
            if(ret2.depth == 0){
                ss << ret2.result_number;
            }
            else{
                ss << "%" << ret2.result_id;
            }
            ss << std::endl;
            return ret;
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

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        if(rule == 1){
            return addexp->Dump2String(ss);
        }
        else{
            ast_ret_t ret;
            ast_ret_t ret1 = relexp->Dump2String(ss);
            ast_ret_t ret2 = addexp->Dump2String(ss);
            ret.result_id = result_id++;
            if(op == "<"){
                ss << "\t%" << ret.result_id << " = lt ";
                ret.result_number = ret1.result_number < ret2.result_number;
            }
            else if(op == ">"){
                ss << "\t%" << ret.result_id << " = gt ";
                ret.result_number = ret1.result_number > ret2.result_number;
            }
            else if(op == "<="){
                ss << "\t%" << ret.result_id << " = le ";
                ret.result_number = ret1.result_number <= ret2.result_number;
            }
            else if(op == ">="){
                ss << "\t%" << ret.result_id << " = ge ";
                ret.result_number = ret1.result_number >= ret2.result_number;
            }
            else{
                assert(false);
            }

            ret.depth = std::max(ret1.depth, ret2.depth) + 1;
            if(ret1.depth == 0){
                ss << ret1.result_number;
            }
            else{
                ss << "%" << ret1.result_id;
            }
            ss << ", ";
            if(ret2.depth == 0){
                ss << ret2.result_number;
            }
            else{
                ss << "%" << ret2.result_id;
            }
            ss << std::endl;
            return ret;
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

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        if(rule == 1){
            return mulexp->Dump2String(ss);
        }
        else{
            ast_ret_t ret;
            ast_ret_t ret1 = addexp->Dump2String(ss);
            ast_ret_t ret2 = mulexp->Dump2String(ss);
            ret.result_id = result_id++;
            switch (op[0])
            {
            case '+':
                ss << "\t%" << ret.result_id << " = add ";
                ret.result_number = ret1.result_number + ret2.result_number;
                break;
            case '-':
                ss << "\t%" << ret.result_id << " = sub ";
                ret.result_number = ret1.result_number - ret2.result_number;
                break;
            default:
                assert(false);
                break;
            }

            ret.depth = std::max(ret1.depth, ret2.depth) + 1;
            if(ret1.depth == 0){
                ss << ret1.result_number;
            }
            else{
                ss << "%" << ret1.result_id;
            }
            ss << ", ";
            if(ret2.depth == 0){
                ss << ret2.result_number;
            }
            else{
                ss << "%" << ret2.result_id;
            }
            ss << std::endl;
            return ret;
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

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        if(rule == 1){
            return unaryexp->Dump2String(ss);
        }
        else{
            ast_ret_t ret;
            ast_ret_t ret1 = mulexp->Dump2String(ss);
            ast_ret_t ret2 = unaryexp->Dump2String(ss);
            ret.result_id = result_id++;
            switch (op[0])
            {
            case '/':
                ss << "\t%" << ret.result_id << " = div ";
                ret.result_number = ret1.result_number / ret2.result_number;
                break;
            case '*':
                ss << "\t%" << ret.result_id << " = mul ";
                ret.result_number = ret1.result_number * ret2.result_number;
                break;
            case '%':
                ss << "\t%" << ret.result_id << " = mod ";
                ret.result_number = ret1.result_number % ret2.result_number;
                break;
            default:
                assert(false);
                break;
            }

            ret.depth = std::max(ret1.depth, ret2.depth) + 1;
            if(ret1.depth == 0){
                ss << ret1.result_number;
            }
            else{
                ss << "%" << ret1.result_id;
            }
            ss << ", ";
            if(ret2.depth == 0){
                ss << ret2.result_number;
            }
            else{
                ss << "%" << ret2.result_id;
            }
            ss << std::endl;
            return ret;
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

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        ast_ret_t ret;
        if(rule == 1){
            ret = primaryexp->Dump2String(ss);
        }
        else if(rule == 2){
            ast_ret_t ret1 = unaryexp->Dump2String(ss);
            switch (unaryop[0])
            {
            case '+':
                /* do nothing */
                return ret1;
                break;
            case '-':
                ss << "\t%" << result_id << " = sub 0, ";
                ret.result_number = -ret1.result_number;
                break;
            case '!':
                ss << "\t%" << result_id << " = eq 0, ";
                ret.result_number = !ret1.result_number;
                break;
            default:
                assert(false);
                break;
            }

            if(ret1.depth == 0){
                ss << ret1.result_number;
            }
            else{
                ss << "%" << ret1.result_id;
            }
            ss << std::endl;

            ret.result_id = result_id++;
            ret.depth = ret.depth + 1;
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
            ret.result_number = number;
            ret.depth = 0;
        }
        return ret;
    }
};

/* Exp         ::= LOrExp; */
class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> lorexp;

    void Dump() const override {
        std::cout << "ExpAST { ";
        lorexp->Dump();
        std::cout << " }";
    }

    ast_ret_t Dump2String(std::stringstream &ss) const override {
        return lorexp->Dump2String(ss);
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
            ss << "\tret " << ret.result_number << std::endl;
        }
        else{
            ss << "\tret %" << ret.result_id << std::endl;
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