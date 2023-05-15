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
// static int block_id = 0;
// static std::map<std::string, SymbolTable> map_func2symbolTable;
// static std::stack<std::string> stack_namespace;

static std::stack<int> stack_while_id;

typedef struct{
    int depth;
    int result_number;
    int result_id;
} exp_result_t;

typedef struct{
    bool lhs;
    bool is_var;
    int val;
    // int result_id;
    std::string pointer;
} l_val_result_t;

// typedef struct{
//     // bool is_end_with_ret;
//     bool is_end_with_if;
// } stmt_result_t;

typedef struct{
    std::string ident;
    std::string b_type;
} func_f_param_result_t;

typedef struct{
    int count;
    std::vector<func_f_param_result_t> params;
} func_f_params_result_t;

typedef struct{
    int count;
    std::vector<exp_result_t> params;
} func_r_params_result_t;

/* Base AST class */
class BaseAST;

/* Part 1: CompUnit */
class StartSymbolAST;
class CompUnitsAST;
class CompUnitAST;

/* Part 2: Decl */
class DeclAST;
class ConstDeclAST;
class BtypeAST;
class ConstDefsAST;
class ConstDefAST;
class ConstInitValAST;
class VarDeclAST;
class VarDefsAST;
class VarDefAST;
class InitValAST;

/* Part 3: Func */
class FuncDefAST;
class FuncTypeAST;
class FuncFParamsAST;
class FuncFParamAST;

/* Part 4: Block */
class BlockAST;
class BlockItemsAST;
class BlockItemAST;
class GeneralStmtAST;
class StmtAST;
class OpenStmtAST;

/* Part 5: Exp */
class ExpAST;
class LValAST;
class PrimaryExpAST;
class UnaryExpAST;
class FuncRParamsAST;
class FuncRParamAST;
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

/* Part 0: StartSymbol */
class StartSymbolAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> comp_units;

    void Dump() const override{
        comp_units->Dump();
        std::cout << std::endl;
    }

    void Dump2StringIR(void *aux) const override {
        /* Some initialization */
        stack_while_id = std::stack<int>();
        stack_namespace = std::stack<int>();

        stack_namespace.push(GLOBAL_NAMESPACE_ID);
        map_blockID2symbolTable[GLOBAL_NAMESPACE_ID] = SymbolTable();
        map_blockID2symbolTable[GLOBAL_NAMESPACE_ID].update_parent_block(ROOT_NAMESPACE_ID);

        SymbolTable::insert_lib_func_def();

        comp_units->Dump2StringIR(nullptr);
    }
};

/* Part 1: CompUnit */
/* CompUnit    ::= [CompUnit] FuncDef; */

class CompUnitsAST: public BaseAST{
public:
    std::vector<std::unique_ptr<BaseAST> > vec_comp_units;

    void Dump() const override {
        std::cout << "CompUnitsAST { ";
        auto ii = vec_comp_units.begin();
        auto ie = vec_comp_units.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        auto ii = vec_comp_units.begin();
        auto ie = vec_comp_units.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump2StringIR(nullptr);
        }
    }
};

class CompUnitAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override {
        std::cout << "CompUnitAST { ";
        func_def->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        func_def->Dump2StringIR(nullptr);
    }
};

/* Part 2: Decl */
/* Decl          ::= ConstDecl | VarDecl; */
class DeclAST : public BaseAST {
public:
    int rule;
    std::unique_ptr<BaseAST> decl;

    void Dump() const override {
        std::cout << "DeclAST { ";
        std::cout << "rule: " << rule << ", ";
        decl->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        decl->Dump2StringIR(nullptr);
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

        int cur_namespace = stack_namespace.top();
        assert(!map_blockID2symbolTable[cur_namespace].bool_symbol_exist_local(ident));

        exp_result_t const_exp_result;
        const_init_val->Dump2StringIR(&const_exp_result);
        assert(const_exp_result.depth == 0);
        int val = const_exp_result.result_number;

        map_blockID2symbolTable[cur_namespace].insert_const_definition_int(ident, val);
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

/* VarDecl       ::= BType VarDef {"," VarDef} ";"; */
class VarDeclAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> b_type;
    std::unique_ptr<BaseAST> var_defs;

    void Dump() const override {
        std::cout << "VarDeclAST { ";
        b_type->Dump();
        var_defs->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        std::string string_b_type;
        b_type->Dump2StringIR(&string_b_type);
        /* TODO: what if other type? */
        assert(string_b_type == "int");

        var_defs->Dump2StringIR(&string_b_type);
    }
};

class VarDefsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_var_defs;

    void Dump() const override {
        std::cout << "VarDefsAST { ";
        auto ii = vec_var_defs.begin();
        auto ie = vec_var_defs.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        auto ii = vec_var_defs.begin();
        auto ie = vec_var_defs.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump2StringIR(aux);
        }
    }
};

/* VarDef        ::= IDENT | IDENT "=" InitVal; */
class VarDefAST : public BaseAST {
public:
    std::string ident;
    std::unique_ptr<BaseAST> init_val;

    void Dump() const override {
        std::cout << "VarDefAST { ";
        std::cout << ident;
        if(init_val != nullptr){
            std::cout << " = ";
            init_val->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        std::string *string_b_type = (std::string *)aux;
        /* TODO: what if other type */
        assert((*string_b_type) == "int");
        std::string string_koopa_type = "i32";

        if(init_val == nullptr){
            int cur_namespace = stack_namespace.top();
            assert(!map_blockID2symbolTable[cur_namespace].bool_symbol_exist_local(ident));

            map_blockID2symbolTable[cur_namespace].insert_var_definition_int(ident, stack_namespace.top());

            std::cout << "\t"
                << map_blockID2symbolTable[cur_namespace].get_var_pointer_int(ident)
                << " = alloc "
                << string_koopa_type
                << std::endl;
        }
        else{
            exp_result_t init_val_result;
            init_val->Dump2StringIR(&init_val_result);

            std::string string_value_to_be_stored;
            if(init_val_result.depth == 0){
                /* the value is in init_val_result.result_number */
                string_value_to_be_stored = std::to_string(init_val_result.result_number);
            }
            else{
                /* the value is in %init_val_result.result_id*/
                string_value_to_be_stored = "%" + std::to_string(init_val_result.result_id);
            }

            int cur_namespace = stack_namespace.top();
            assert(!map_blockID2symbolTable[cur_namespace].bool_symbol_exist_local(ident));

            map_blockID2symbolTable[cur_namespace].insert_var_definition_int(ident, stack_namespace.top());

            std::string string_var_pointer = map_blockID2symbolTable[cur_namespace].get_var_pointer_int(ident);

            std::cout << "\t"
                << string_var_pointer
                << " = alloc "
                << string_koopa_type
                << std::endl;

            std::cout << "\tstore "
                << string_value_to_be_stored
                << ", " << string_var_pointer
                << std::endl;
        }
    }
};

/* InitVal       ::= Exp; */
class InitValAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << "InitValAST { ";
        exp->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        exp->Dump2StringIR(aux);
    }
};

/* Part 3: Func */
/* FuncDef     ::= FuncType IDENT "(" [FuncFParams] ")" Block; */
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> func_f_params;
    std::unique_ptr<BaseAST> block;

    void Dump() const override {
        std::cout << "FuncDefAST { ";
        func_type->Dump();
        std::cout << ", " << ident << ", ";
        if(func_f_params == nullptr){
            std::cout << "params_null, ";
        }
        else{
            func_f_params->Dump();
        }
        block->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        /* TODO: params check? return type check? */
        // map_blockID2symbolTable[ident] = SymbolTable();
        // stack_namespace.push(ident);
        std::cout << "fun ";
        std::cout << "@" << ident << "(";

        int cur_namespace = stack_namespace.top();

        func_f_params_result_t func_f_params_result;
        if(func_f_params == nullptr){
            func_f_params_result.count = 0;
        }
        else{
            func_f_params->Dump2StringIR(&func_f_params_result);
        }

        std::cout << ")";

        std::string string_type;
        func_type->Dump2StringIR(&string_type);

        map_blockID2symbolTable[cur_namespace].insert_func_def(ident, string_type);

        std::cout << " {" << std::endl;
        std::cout << "%" << "entry" << ":" << std::endl;

        /* TODO: may pass some func related info to its block? */
        block->Dump2StringIR(&func_f_params_result);

        std::cout << "\tret" << std::endl;
        std::cout << "}" << std::endl;
        // stack_namespace.pop();
        // map_blockID2symbolTable[ident].clear_table();
    }
};

/* FuncType    ::= "void" | "int"; */
class FuncTypeAST : public BaseAST {
public:
    std::string type_string;

    void Dump() const override {
        std::cout << "FuncTypeAST { ";
        std::cout << type_string;
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        assert(type_string == "int" || type_string == "void");
        *((std::string *)aux) = type_string;
        if(type_string == "int"){
            std::cout << ": i32";
        }
    }
};

/* FuncFParams ::= FuncFParam {"," FuncFParam}; */
class FuncFParamsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_func_f_params;

    void Dump() const override {
        std::cout << "FuncFParamsAST { ";
        auto ii = vec_func_f_params.begin();
        auto ie = vec_func_f_params.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        func_f_params_result_t *params_r = (func_f_params_result_t *)aux;
        params_r->count = 0;
        params_r->params = std::vector<func_f_param_result_t>();
        auto ii = vec_func_f_params.begin();
        auto ie = vec_func_f_params.end();
        if(ii != ie){
            func_f_param_result_t param_r;
            (*ii)->Dump2StringIR(&param_r);
            params_r->count++;
            params_r->params.push_back(param_r);
            ++ii;
        }
        for(; ii != ie; ++ii){
            func_f_param_result_t param_r;
            std::cout << ", ";
            (*ii)->Dump2StringIR(&param_r);
            params_r->count++;
            params_r->params.push_back(param_r);
        }
    }
};

/* FuncFParam  ::= BType IDENT; */
class FuncFParamAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> b_type;
    std::string ident;

    void Dump() const override {
        std::cout << "FuncFParamAST { ";
        b_type->Dump();
        std::cout <<  " " << ident;
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        func_f_param_result_t *param_r = (func_f_param_result_t *)aux;
        std::string string_b_type;
        b_type->Dump2StringIR(&string_b_type);

        /* TODO: what if other types */
        assert(string_b_type == "int");
        std::cout << "@" << ident << ": " << "i32";

        param_r->b_type = string_b_type;
        param_r->ident = ident;
    }
};

/* Part 4: Block */
/* Block         ::= "{" {BlockItem} "}"; */
class BlockAST : public BaseAST {
public:
    int id;
    std::unique_ptr<BaseAST> block_items;

    void Dump() const override {
        std::cout << "BlockAST { ";
        if(block_items != nullptr){
            block_items->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        /* might be nullptr*/
        func_f_params_result_t *params = (func_f_params_result_t *)aux;

        // stmt_result_t *last_stmt_result = (stmt_result_t *)aux;
        map_blockID2symbolTable[id] = SymbolTable();

        /* TODO: modify here after introducing global var */
        // if(stack_namespace.empty()){
        //     map_blockID2symbolTable[id].update_parent_block(-1);
        // }
        // else{
            assert(!stack_namespace.empty());
            map_blockID2symbolTable[id].update_parent_block(stack_namespace.top());
        // }
        stack_namespace.push(id);

        if(params != nullptr){
            int count = params->count;
            for(int i = 0; i < count; ++i){
                std::string &ident = params->params[i].ident;
                assert(!map_blockID2symbolTable[id].bool_symbol_exist_local(ident));

                assert(params->params[i].b_type == "int");
                map_blockID2symbolTable[id].insert_var_func_param_int(ident, id);

                std::cout << "\t" << map_blockID2symbolTable[id].get_var_pointer_int(ident);
                std::cout << "= alloc i32" << std::endl;

                std::cout << "\tstore @" << ident << ", ";
                std::cout << map_blockID2symbolTable[id].get_var_pointer_int(ident);
                std::cout << std::endl;
            }
        }

        if(block_items != nullptr){
            block_items->Dump2StringIR(nullptr);
        }
        // else{
        //     if(last_stmt_result != nullptr){
        //         // last_stmt_result->is_end_with_ret = false;
        //         last_stmt_result->is_end_with_if = false;
        //     }
        // }

        stack_namespace.pop();
        map_blockID2symbolTable[id].clear_table();
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
        // stmt_result_t *last_stmt_result = (stmt_result_t *)aux;
        // if(last_stmt_result != nullptr){
        //     // last_stmt_result->is_end_with_ret = false;
        //     last_stmt_result->is_end_with_if = false;
        // }

        auto ii = vec_block_items.begin();
        auto ie = vec_block_items.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump2StringIR(nullptr);
        }
    }
};

/* BlockItem     ::= Decl | GeneralStmt; */
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
        // stmt_result_t *last_stmt_result = (stmt_result_t *)aux;
        if(is_stmt){
            item->Dump2StringIR(nullptr);
        }
        else{
            item->Dump2StringIR(nullptr);
            // stmt_result_t *stmt_result = (stmt_result_t *)aux;
            // if(stmt_result != nullptr){
            //     stmt_result->is_end_with_ret = false;
            //     stmt_result->is_end_with_if = false;
            // }
        }
    }
};

/* GeneralStmt     ::= Stmt | OpenStmt; */
class GeneralStmtAST : public BaseAST {
public:
    bool is_open;
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override {
        std::cout << "GeneralStmtAST { ";
        std::cout << "is_open: " << is_open << ", ";
        stmt->Dump();
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        // stmt_result_t *last_stmt_result = (stmt_result_t *)aux;
        stmt->Dump2StringIR(nullptr);
    }
};


/*
    Stmt ::= LVal "=" Exp ";"
        | [Exp] ";"
        | Block
        | "return" [Exp] ";";
        | IF '(' Exp ')' Stmt ELSE Stmt
*/
class StmtAST : public BaseAST {
public:
    int rule;
    int if_stmt_id;
    int ret_id;
    int while_id;
    std::unique_ptr<BaseAST> l_val;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> block;
    std::unique_ptr<BaseAST> stmt_true;
    std::unique_ptr<BaseAST> stmt_false;
    std::unique_ptr<BaseAST> stmt_body;


    void Dump() const override {
        std::cout << "StmtAST { ";

        if(rule == 1){
            l_val->Dump();
            std::cout << " = ";
            exp->Dump();
            std::cout << "; }";
        }
        else if(rule == 2){
            if(exp != nullptr){
                exp->Dump();
            }
            std::cout << "; }";
        }
        else if(rule == 3){
            block->Dump();
        }
        else if(rule == 4){
            std::cout << "return ";
            if(exp != nullptr){
                exp->Dump();
            }
            std::cout << "; }";
        }
        else if(rule == 5){
            std::cout << "if ( ";
            exp->Dump();
            std::cout << ") ";
            stmt_true->Dump();
            std::cout << " else ";
            stmt_false->Dump();
            std::cout << " }";
        }
        else if(rule == 6){
            std::cout << "while ( ";
            exp->Dump();
            std::cout << ") ";
            stmt_body->Dump();
            std::cout << " }";
        }
        else if(rule == 7){
            std::cout << "break ; }";
        }
        else if(rule == 8){
            std::cout << "continue ; }";
        }
        else{
            assert(false);
        }
    }

    void Dump2StringIR(void *aux) const override {
        // stmt_result_t *stmt_result = (stmt_result_t *)aux;
        // if(stmt_result != nullptr){
        //     // stmt_result->is_end_with_ret = false;
        //     stmt_result->is_end_with_if = false;
        // }

        if(rule == 1){
            exp_result_t exp_result;
            exp->Dump2StringIR(&exp_result);

            l_val_result_t l_val_result;
            l_val_result.lhs = true;
            l_val->Dump2StringIR(&l_val_result);

            assert(l_val_result.is_var);
            if(exp_result.depth == 0){
                std::cout << "\tstore " << exp_result.result_number
                        << ", " << l_val_result.pointer << std::endl;
            }
            else{
                std::cout << "\tstore %" << exp_result.result_id
                        << ", " << l_val_result.pointer << std::endl;
            }
        }
        else if(rule == 2){
            if(exp != nullptr){
                exp_result_t exp_result;
                exp->Dump2StringIR(&exp_result);
            }
        }
        else if(rule == 3){
            block->Dump2StringIR(nullptr);
        }
        else if(rule == 4){
            // if(stmt_result != nullptr){
            //     stmt_result->is_end_with_ret = true;
            // }
            /* TODO: what if no return value */
            std::cout << "\tjump %ret_" << ret_id << std::endl;
            std::cout << "%ret_" << ret_id << ":" << std::endl;
            if(exp == nullptr){
                std::cout << "\tret" << std::endl;
            }
            else{
                exp_result_t exp_result;
                exp->Dump2StringIR(&exp_result);

                if(exp_result.depth == 0){
                    std::cout << "\tret " << exp_result.result_number << std::endl;
                }
                else{
                    std::cout << "\tret %" << exp_result.result_id << std::endl;
                }
            }
            std::cout << "%after_ret_" << ret_id << ":" << std::endl;
        }
        else if(rule == 5){
            // stmt_result_t stmt_result_1, stmt_result_2;
            exp_result_t exp_result;
            exp->Dump2StringIR(&exp_result);
            if(exp_result.depth != 0){
                std::cout << "\tbr %" << exp_result.result_id << ", ";
            }
            else{
                std::cout << "\tbr " << exp_result.result_number << ", ";
            }
            std::cout << "%then_" << if_stmt_id << ", ";
            std::cout << "%else_" << if_stmt_id << std::endl;

            std::cout << "%then_" << if_stmt_id << ":" << std::endl;
            stmt_true->Dump2StringIR(nullptr);
            // if(!stmt_result_1.is_end_with_ret){
                std::cout << "\tjump %end_" << if_stmt_id << std::endl;
            // }

            std::cout << "%else_" << if_stmt_id << ":" << std::endl;
            stmt_false->Dump2StringIR(nullptr);
            // if(!stmt_result_2.is_end_with_ret || stmt_result_2.is_end_with_if){
                std::cout << "\tjump %end_" << if_stmt_id << std::endl;
            // }

            // if(!(stmt_result_1.is_end_with_ret && stmt_result_2.is_end_with_ret)){
                std::cout << "%end_" << if_stmt_id << ":" << std::endl;
            // }
            // if(stmt_result != nullptr){
            //     // stmt_result->is_end_with_ret = stmt_result_1.is_end_with_ret
            //     //                             && stmt_result_2.is_end_with_ret;
            //     stmt_result->is_end_with_if = true;
            // }

        }
        else if(rule == 6){
            exp_result_t exp_result;
            // stmt_result_t stmt_body_result;

            stack_while_id.push(while_id);

            std::cout << "\tjump %while_cond_" << while_id << std::endl;

            std::cout << "%while_cond_" << while_id << ":" << std::endl;
            exp->Dump2StringIR(&exp_result);
            if(exp_result.depth == 0){
                std::cout << "\tbr " << exp_result.result_number << ", ";
            }
            else{
                std::cout << "\tbr %" << exp_result.result_id << ", ";
            }
            std::cout << "%while_body_" << while_id << ", ";
            std::cout << "%while_end_" << while_id << std::endl;

            std::cout << "%while_body_" << while_id << ":" << std::endl;
            /* TODO: emmm, stmt_result_t seems unnecessary now? */
            stmt_body->Dump2StringIR(nullptr);
            std::cout << "\tjump %while_cond_" << while_id << std::endl;

            std::cout << "%while_end_" << while_id << ":" << std::endl;

            stack_while_id.pop();
        }
        else if(rule == 7){
            assert(!stack_while_id.empty());
            int target = stack_while_id.top();

            std::cout << "\tjump %while_end_" << target << std::endl;
            std::cout << "%after_break_while_" << target << ":";
            std::cout << std::endl;
        }
        else if(rule == 8){
            assert(!stack_while_id.empty());
            int target = stack_while_id.top();

            std::cout << "\tjump %while_cond_" << target << std::endl;
            std::cout << "%after_continue_while_" << target << ":";
            std::cout << std::endl;
        }
        else{
            assert(false);
        }
    }
};

/*
    OpenStmt    ::= IF '(' Exp ')' GeneralStmt
                | IF '(' Exp ')' Stmt ELSE OpenStmt
*/
class OpenStmtAST : public BaseAST {
public:
    bool is_with_else;
    int if_stmt_id;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> stmt_true;
    std::unique_ptr<BaseAST> stmt_false;

    void Dump() const override {
        std::cout << "OpenStmtAST { ";
        std::cout << "if ( ";
        exp->Dump();
        std::cout << ") ";
        std::cout << "is_with_else: " << is_with_else << ", ";
        stmt_true->Dump();
        if(is_with_else){
            std::cout << " else ";
            stmt_false->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        // stmt_result_t *stmt_result = (stmt_result_t *)aux;
        // if(stmt_result != nullptr){
        //     // stmt_result->is_end_with_ret = false;
        //     stmt_result->is_end_with_if = true;
        // }

        // stmt_result_t stmt_result_1, stmt_result_2;
        exp_result_t exp_result;
        exp->Dump2StringIR(&exp_result);
        if(exp_result.depth != 0){
            std::cout << "\tbr %" << exp_result.result_id << ", ";
        }
        else{
            std::cout << "\tbr " << exp_result.result_number << ", ";
        }
        std::cout << "%then_" << if_stmt_id << ", ";
        if(is_with_else){
            std::cout << "%else_" << if_stmt_id << std::endl;
        }
        else{
            std::cout << "%end_" << if_stmt_id << std::endl;
        }

        std::cout << "%then_" << if_stmt_id << ":" << std::endl;
        stmt_true->Dump2StringIR(nullptr);

        if(is_with_else){
            // if(!stmt_result_1.is_end_with_ret){
                std::cout << "\tjump %end_" << if_stmt_id << std::endl;
            // }

            std::cout << "%else_" << if_stmt_id << ":" << std::endl;
            stmt_false->Dump2StringIR(nullptr);
            // if(!stmt_result_2.is_end_with_ret || stmt_result_2.is_end_with_if){
                std::cout << "\tjump %end_" << if_stmt_id << std::endl;
            // }

            // if(!(stmt_result_1.is_end_with_ret && stmt_result_2.is_end_with_ret)){
                std::cout << "%end_" << if_stmt_id << ":" << std::endl;
            // }

            // if(stmt_result != nullptr){
            //     stmt_result->is_end_with_ret = stmt_result_1.is_end_with_ret
            //                                 && stmt_result_2.is_end_with_ret;
            // }
        }
        else{
            // if(!stmt_result_1.is_end_with_ret || stmt_result_1.is_end_with_if){
                std::cout << "\tjump %end_" << if_stmt_id << std::endl;
            // }

            // if(!stmt_result_1.is_end_with_ret){
            std::cout << "%end_" << if_stmt_id << ":" << std::endl;
            // }
            // if(stmt_result != nullptr){
            //     stmt_result->is_end_with_ret = stmt_result_1.is_end_with_ret;
            // }
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
        int cur_namespace = stack_namespace.top();

        if(((l_val_result_t *)aux)->lhs){
            assert(map_blockID2symbolTable[cur_namespace].bool_symbol_is_var(ident));
            ((l_val_result_t *)aux)->is_var = true;
            ((l_val_result_t *)aux)->pointer =
                map_blockID2symbolTable[cur_namespace].get_var_pointer_int(ident);
        }
        else{
            bool is_var = map_blockID2symbolTable[cur_namespace].bool_symbol_is_var(ident);
            ((l_val_result_t *)aux)->is_var = is_var;
            if(is_var){
                ((l_val_result_t *)aux)->pointer =
                    map_blockID2symbolTable[cur_namespace].get_var_pointer_int(ident);

                std::cout << "\t%" << result_id++ << " = load "
                        << ((l_val_result_t *)aux)->pointer
                        << std::endl;
            }
            else{
                ((l_val_result_t *)aux)->val =
                    map_blockID2symbolTable[cur_namespace].get_const_definition_int(ident);
            }
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

            if(l_val_result.is_var){
                ((exp_result_t *)aux)->depth = 1;
                ((exp_result_t *)aux)->result_id = result_id - 1;
            }
            else{

                ((exp_result_t *)aux)->depth = 0;
                ((exp_result_t *)aux)->result_number = l_val_result.val;
            }
        }
    }
};

/*
    UnaryExp    ::= PrimaryExp | IDENT "(" [FuncRParams] ")" | UnaryOp UnaryExp;
    UnaryOp     ::= "+" | "-" | "!";
*/
class UnaryExpAST : public BaseAST {
public:
    int rule;
    std::unique_ptr<BaseAST> primaryexp;
    std::string unaryop;
    std::unique_ptr<BaseAST> unaryexp;
    std::string ident;
    std::unique_ptr<BaseAST> func_r_params;

    void Dump() const override {
        std::cout << "UnaryExpAST { ";
        if(rule == 1){
            primaryexp->Dump();
        }
        else if(rule == 2){
            std::cout << unaryop;
            unaryexp->Dump();
        }
        else if(rule == 3){
            std::cout << ident << " ( ";
            if(func_r_params != nullptr){
                func_r_params->Dump();
            }
            std::cout << " )";
        }
        else{
            assert(false);
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
        else if(rule == 3){
            exp_result_t *result = (exp_result_t *)aux;
            func_r_params_result_t params;

            if(func_r_params != nullptr){
                func_r_params->Dump2StringIR(&params);
            }

            result->depth = 1;

            int cur_namespace = stack_namespace.top();

            std::string type_return = map_blockID2symbolTable[cur_namespace].get_func_return_type(ident);

            if(type_return == "void"){
                std::cout << "\tcall @" << ident << "(";
            }
            else if(type_return == "int"){
                result->result_id = result_id++;
                std::cout << "\t%" << result->result_id << " = call @";
                std::cout << ident << "(";
            }
            else{
                assert(false);
            }

            if(func_r_params != nullptr){
                int i = 0;
                int count = params.count;
                if(i != count){
                    if(params.params[i].depth == 0){
                        std::cout << params.params[i].result_number;
                    }
                    else{
                        std::cout << "%" << params.params[i].result_id;
                    }
                    ++i;
                }
                for(; i < params.count; ++i){
                    std::cout << ", ";
                    if(params.params[i].depth == 0){
                        std::cout << params.params[i].result_number;
                    }
                    else{
                        std::cout << "%" << params.params[i].result_id;
                    }
                }
            }

            std::cout << ")" << std::endl;
        }
        else{
            assert(false);
        }
    }
};

/* FuncRParams ::= Exp {"," Exp}; */
class FuncRParamsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_func_r_params;

    void Dump() const override {
        std::cout << "FuncRParamsAST { ";
        auto ii = vec_func_r_params.begin();
        auto ie = vec_func_r_params.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " }";
    }

    void Dump2StringIR(void *aux) const override {
        func_r_params_result_t *params = (func_r_params_result_t *)aux;
        params->count = 0;
        params->params = std::vector<exp_result_t>();

        auto ii = vec_func_r_params.begin();
        auto ie = vec_func_r_params.end();
        for(; ii != ie; ++ii){
            /* TODO */
            exp_result_t exp_result;
            (*ii)->Dump2StringIR(&exp_result);
            params->count++;
            params->params.push_back(exp_result);
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
                    std::cout << "\t%" << result->result_id << " = div ";
                    break;
                case '*':
                    std::cout << "\t%" << result->result_id << " = mul ";
                    break;
                case '%':
                    std::cout << "\t%" << result->result_id << " = mod ";
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
    int id;
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

            /* short-circuit logic */
            if(result1.depth == 0){
                if(result1.result_number == 0){
                    result->depth = 0;
                    result->result_number = 0;
                }
                else{
                    eqexp->Dump2StringIR(&result2);
                    if(result2.depth == 0){
                        result->depth = 0;
                        result->result_number = result1.result_number
                                            && result2.result_number;
                    }
                    else{
                        result->depth = result2.depth + 1;
                        result->result_id = result_id++;
                        std::cout << "\t%" << result->result_id;
                        std::cout << " = ne 0, %" << result2.result_id;
                        std::cout << std::endl;
                    }
                }
            }
            else{
                std::cout << "\t%tmp_l_and_exp_" << id;
                std::cout << " = alloc i32" << std::endl;

                std::cout << "\tstore 0, %tmp_l_and_exp_" << id;
                std::cout << std::endl;

                std::cout << "\tbr %" << result1.result_id << ", ";
                std::cout << "%then_l_and_exp_" << id << ", ";
                std::cout << "%end_l_and_exp_" << id << std::endl;

                std::cout << "%then_l_and_exp_" << id << ":" << std::endl;

                eqexp->Dump2StringIR(&result2);

                std::cout << "\t%" << result_id++ << " = ne 0, ";
                std::cout << "%" << result1.result_id << std::endl;

                std::cout << "\t%" << result_id++ << " = ne 0, ";
                if(result2.depth == 0){
                    std::cout << result2.result_number;
                }
                else{
                    std::cout << "%" << result2.result_id;
                }
                std::cout << std::endl;

                /*
                    TODO: the semantics of `depth` is problematic here.
                    better change from int to bool
                */
                result->depth = std::max(result1.depth, result2.depth) + 1;
                result->result_id = result_id++;

                std::cout   << "\t%" << result->result_id << " = and %"
                            << result->result_id - 1 << ", %"
                            << result->result_id - 2
                            << std::endl;

                std::cout << "\t store %" << result->result_id;
                std::cout << ", %tmp_l_and_exp_" << id << std::endl;

                std::cout << "\tjump %end_l_and_exp_" << id << std::endl;

                std::cout << "%end_l_and_exp_" << id << ":" << std::endl;

                result->result_id = result_id++;
                std::cout << "\t %" << result->result_id << " = load ";
                std::cout << "%tmp_l_and_exp_" << id << std::endl;
            }

            // eqexp->Dump2StringIR(&result2);

            // if(result1.depth == 0 && result2.depth == 0){
            //     result->depth = 0;
            //     result->result_number = result1.result_number
            //                             && result2.result_number;
            // }
            // else{
            //     std::cout << "\t%" << result_id++ << " = ne 0, ";
            //     if(result1.depth == 0){
            //         std::cout << result1.result_number;
            //     }
            //     else{
            //         std::cout << "%" << result1.result_id;
            //     }
            //     std::cout << std::endl;

            //     std::cout << "\t%" << result_id++ << " = ne 0, ";
            //     if(result2.depth == 0){
            //         std::cout << result2.result_number;
            //     }
            //     else{
            //         std::cout << "%" << result2.result_id;
            //     }
            //     std::cout << std::endl;

            //     result->depth = std::max(result1.depth, result2.depth) + 1;
            //     result->result_id = result_id++;

            //     std::cout   << "\t%" << result->result_id << " = and %"
            //                 << result->result_id - 1 << ", %"
            //                 << result->result_id - 2
            //                 << std::endl;
            // }
        }
    }
};

/* LOrExp      ::= LAndExp | LOrExp "||" LAndExp; */
class LOrExpAST : public BaseAST {
public:
    int rule;
    int id;
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

            /* short-circuit logic */
            if(result1.depth == 0){
                if(result1.result_number != 0){
                    result->depth = 0;
                    result->result_number = 1;
                }
                else{
                    landexp->Dump2StringIR(&result2);
                    if(result2.depth == 0){
                        result->depth = 0;
                        result->result_number = result1.result_number
                                            || result2.result_number;
                    }
                    else{
                        result->depth = result2.depth + 1;
                        result->result_id = result_id++;
                        std::cout << "\t%" << result->result_id;
                        std::cout << " = ne 0, %" << result2.result_id;
                        std::cout << std::endl;
                    }
                }
            }
            else{
                std::cout << "\t%tmp_l_or_exp_" << id;
                std::cout << " = alloc i32" << std::endl;

                std::cout << "\tstore 1, %tmp_l_or_exp_" << id;
                std::cout << std::endl;

                std::cout << "\tbr %" << result1.result_id << ", ";
                std::cout << "%end_l_or_exp_" << id << ", ";
                std::cout << "%then_l_or_exp_" << id << std::endl;

                std::cout << "%then_l_or_exp_" << id << ":" << std::endl;

                landexp->Dump2StringIR(&result2);

                std::cout << "\t%" << result_id++ << " = or ";
                std::cout << "%" << result1.result_id << ", ";
                if(result2.depth == 0){
                    std::cout << result2.result_number;
                }
                else{
                    std::cout << "%" << result2.result_id;
                }
                std::cout << std::endl;

                /*
                    TODO: the semantics of `depth` is problematic here.
                    better change from int to bool
                */
                result->depth = std::max(result1.depth, result2.depth) + 1;
                result->result_id = result_id++;
                std::cout << "\t%" << result->result_id << " = ne 0, %";
                std::cout << result->result_id - 1 << std::endl;

                std::cout << "\tstore %" << result->result_id;
                std::cout << ", %tmp_l_or_exp_" << id << std::endl;

                std::cout << "\tjump %end_l_or_exp_" << id << std::endl;

                std::cout << "%end_l_or_exp_" << id << ":" << std::endl;

                result->result_id = result_id++;
                std::cout << "\t%" << result->result_id << " = load ";
                std::cout << "%tmp_l_or_exp_" << id << std::endl;
            }

            // landexp->Dump2StringIR(&result2);

            // if(result1.depth == 0 && result2.depth == 0){
            //     result->depth = 0;
            //     result->result_number = result1.result_number
            //                             || result2.result_number;
            // }
            // else{
            //     result->depth = std::max(result1.depth, result2.depth) + 1;
            //     result->result_id = result_id++;

            //     std::cout << "\t%" << result->result_id << " = or ";
            //     if(result1.depth == 0){
            //         std::cout << result1.result_number;
            //     }
            //     else{
            //         std::cout << "%" << result1.result_id;
            //     }
            //     std::cout << ", ";
            //     if(result2.depth == 0){
            //         std::cout << result2.result_number;
            //     }
            //     else{
            //         std::cout << "%" << result2.result_id;
            //     }
            //     std::cout << std::endl;

            //     result->result_id = result_id++;
            //     std::cout << "\t%" << result->result_id << " = ne 0, %";
            //     std::cout << result->result_id - 1 << std::endl;
            // }
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
