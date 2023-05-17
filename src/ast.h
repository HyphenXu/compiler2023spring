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
#include "type.h"

static int result_id = 0;
static std::stack<int> stack_while_id;

typedef struct{
    bool is_zero_depth;
    int result_number;
    int result_id;
} exp_result_t;

typedef struct{
    bool lhs;
    bool is_var;
    int val;
    std::string pointer;
} l_val_result_t;

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

typedef struct{
    bool is_global;
    std::string b_type;
} decl_param_t;

typedef struct{
    int array_length;
} const_exp_array_result_t;

typedef struct{
    bool is_array;

    int val;

    int length;
    std::vector<int> init_val;
} const_init_val_result_t;

typedef struct{
    bool is_array;

    exp_result_t exp_result;

    int length;
    std::vector<exp_result_t> exp_result_array;
} init_val_result_t;

typedef struct{
    exp_result_t exp_idx;
} exp_array_result_t;

/* Base AST class */
class BaseAST;

/* Part 0: StartSymbol */
class StartSymbolAST;

/* Part 1: CompUnit */
class CompUnitsAST;
class CompUnitAST;

/* Part 2: Decl */
class DeclAST;
class ConstDeclAST;
class ConstDefsAST;
class ConstDefAST;      /*TODO*/
class ConstInitValAST;  /*TODO*/
class ConstInitValArrayAST;
class ConstExpArrayAST;
class VarDeclAST;
class VarDefsAST;
class VarDefAST;        /*TODO*/
class InitValAST;       /*TODO*/
class InitValArrayAST;

/* Part 3: Func */
class FuncDefAST;
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
class LValAST;          /*TODO*/
class ExpArrayAST;
class PrimaryExpAST;
class UnaryExpAST;
class FuncRParamsAST;
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

    void Dump() const override {
        comp_units->Dump();
        std::cout << std::endl;
    }

    void Dump2StringIR(void *aux [[maybe_unused]]) const override {
        /* Some initialization */
        stack_while_id = std::stack<int>();
        stack_namespace = std::stack<int>();

        stack_namespace.push(GLOBAL_NAMESPACE_ID);
        symbol_tables[GLOBAL_NAMESPACE_ID] = SymbolTable(ROOT_NAMESPACE_ID);

        SymbolTable::insert_lib_func_def();

        comp_units->Dump2StringIR(nullptr);
    }
};

/* Part 1: CompUnit */
/* CompUnit    ::= [CompUnit] FuncDef; */
class CompUnitsAST: public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_comp_units;

    void Dump() const override {
        std::cout << " CompUnitsAST { ";
        auto ii = vec_comp_units.begin();
        auto ie = vec_comp_units.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux [[maybe_unused]]) const override {
        auto ii = vec_comp_units.begin();
        auto ie = vec_comp_units.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump2StringIR(nullptr);
        }
    }
};

class CompUnitAST : public BaseAST {
public:
    comp_unit_type_t type;
    std::unique_ptr<BaseAST> comp_unit;

    void Dump() const override {
        std::cout << " CompUnitAST { ";
        std::cout << " type: " << type << ", ";
        comp_unit->Dump();
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux [[maybe_unused]]) const override {
        decl_param_t param;
        switch (type)
        {
        case COMP_UNIT_FUNC_DEF:
            comp_unit->Dump2StringIR(nullptr);
            break;
        case COMP_UNIT_DECL:
            param.is_global = true;
            comp_unit->Dump2StringIR(&param);
            break;
        default:
            assert(false);
            break;
        }
    }
};

/* Part 2: Decl */
/* Decl          ::= ConstDecl | VarDecl; */
class DeclAST : public BaseAST {
public:
    decl_type_t type; /* TODO: may be unnecessary */
    std::unique_ptr<BaseAST> decl;

    void Dump() const override {
        std::cout << " DeclAST { ";
        std::cout << " type: " << type << ", ";
        decl->Dump();
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        // switch (type)
        // {
        // case DECL_CONST_DECL:
        //     decl->Dump2StringIR(nullptr);
        //     break;
        // case DECL_VAR_DECL:
        //     decl->Dump2StringIR(aux);
        //     break;
        // default:
        //     assert(false);
        //     break;
        // }
        decl->Dump2StringIR(aux);
    }
};

/* ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";"; */
class ConstDeclAST : public BaseAST {
public:
    std::string b_type;
    std::unique_ptr<BaseAST> const_defs;

    void Dump() const override {
        std::cout << " ConstDeclAST { ";
        std::cout << " BType: " << b_type << ", ";
        const_defs->Dump();
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        /* FURTHER: what if other type? */
        assert(b_type == "int");

        decl_param_t *param = (decl_param_t *)aux;
        param->b_type = b_type;
        const_defs->Dump2StringIR(aux);
    }
};

class ConstDefsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_const_defs;

    void Dump() const override {
        std::cout << " ConstDefsAST { ";
        auto ii = vec_const_defs.begin();
        auto ie = vec_const_defs.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " } ";
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
    std::unique_ptr<BaseAST> const_exp_array;
    std::unique_ptr<BaseAST> const_init_val;

    void Dump() const override {
        std::cout << " ConstDefAST { ";
        std::cout << ident;
        if(const_exp_array != nullptr){
            const_exp_array->Dump();
        }
        std::cout << " = ";
        const_init_val->Dump();
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        decl_param_t *param = (decl_param_t *)aux;
        /* FURTHER: what if other type */
        assert(param->b_type == "int");

        int cur_namespace = stack_namespace.top();
        assert(!symbol_tables[cur_namespace].bool_symbol_exist_local(ident));

        const_init_val_result_t civr;
        const_init_val->Dump2StringIR(&civr);

        if(const_exp_array == nullptr){
            // exp_result_t const_exp_result;
            // const_init_val->Dump2StringIR(&const_exp_result);
            // assert(const_exp_result.is_zero_depth);
            // int val = const_exp_result.result_number;
            assert(!civr.is_array);
            int val = civr.val;

            symbol_tables[cur_namespace].insert_const_definition_int(ident, val);
        }
        else{
            const_exp_array_result_t cear;
            const_exp_array->Dump2StringIR(&cear);
            int array_length = cear.array_length;

            symbol_tables[cur_namespace].insert_array_definition_int(ident, stack_namespace.top());

            if(param->is_global){
                std::cout << "global " << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                std::cout << " = alloc ";
                std::cout << "[";
                std::cout << "i32";
                std::cout << ", " << array_length << "]";
                std::cout << ", ";
                if(civr.length == 0){
                    std::cout << "zeroinit";
                }
                else{
                    std::cout << "{";
                    for(int i = 0; i < array_length; ++i){
                        if(i){
                            std::cout << ", ";
                        }
                        if(i < civr.length){
                            std::cout << civr.init_val[i];
                        }
                        else{
                            std::cout << 0;
                        }
                    }
                    std::cout << "}";
                }
                std::cout << std::endl;
            }
            else{
                std::cout << "\t" << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                std::cout << " = alloc ";
                std::cout << "[";
                std::cout << "i32";
                std::cout << ", " << array_length << "]" << std::endl;

                for(int i = 0; i < array_length; ++i){
                    std::cout << "\t%" << result_id++ << " = getelemptr";
                    std::cout << "@" << ident << "_" << cur_namespace;
                    std::cout << ", " << i << std::endl;
                    if(i < civr.length){
                        std::cout << "store " << civr.init_val[i];
                    }
                    else{
                        std::cout << "store 0";
                    }
                    std::cout << " , %" << result_id - 1 << std::endl;
                }
            }
        }
    }
};

/* ConstInitVal  ::= ConstExp; */
class ConstInitValAST : public BaseAST {
public:
    int type;
    std::unique_ptr<BaseAST> const_exp;
    std::unique_ptr<BaseAST> const_init_val_array;

    void Dump() const override {
        std::cout << " ConstInitValAST { ";
        std::cout << " type: " << type << ", ";
        if(type == 0){
            const_exp->Dump();
        }
        else{
            std::cout << " { ";
            if(const_init_val_array != nullptr){
                const_init_val_array->Dump();
            }
            std::cout << " } ";
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        const_init_val_result_t *civr = (const_init_val_result_t *)aux;
        if(type == 0){
            civr->is_array = false;

            exp_result_t const_exp_result;
            const_exp->Dump2StringIR(&const_exp_result);
            assert(const_exp_result.is_zero_depth);
            civr->val = const_exp_result.result_number;
        }
        else{
            civr->is_array = true;

            if(const_init_val_array == nullptr){
                civr->length = 0;
            }
            else{
                const_init_val_array->Dump2StringIR(civr);
            }
        }
    }
};

class ConstInitValArrayAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_const_exps;

    void Dump() const override {
        std::cout << " ConstInitValArrayAST { ";
        auto ii = vec_const_exps.begin();
        auto ie = vec_const_exps.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        const_init_val_result_t *civr = (const_init_val_result_t *)aux;
        civr->init_val = std::vector<int>();
        civr->length = 0;
        auto ii = vec_const_exps.begin();
        auto ie = vec_const_exps.end();
        for(; ii != ie; ++ii){
            exp_result_t exp_result;
            (*ii)->Dump2StringIR(&exp_result);
            assert(exp_result.is_zero_depth);
            civr->init_val.push_back(exp_result.result_number);
            civr->length++;
        }
    }
};

class ConstExpArrayAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> const_exp;

    void Dump() const override {
        std::cout << " ConstExpArrayAST { ";
        std::cout << " [ ";
        const_exp->Dump();
        std::cout << " ] " << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        const_exp_array_result_t *cear = (const_exp_array_result_t *)aux;
        exp_result_t const_exp_result;
        const_exp->Dump2StringIR(&const_exp_result);
        assert(const_exp_result.is_zero_depth);
        cear->array_length = const_exp_result.result_number;
    }
};

/* VarDecl       ::= BType VarDef {"," VarDef} ";"; */
class VarDeclAST : public BaseAST {
public:
    std::string b_type;
    std::unique_ptr<BaseAST> var_defs;

    void Dump() const override {
        std::cout << " VarDeclAST { ";
        std::cout << " BType: " << b_type << ", ";
        var_defs->Dump();
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        /* FURTHER: what if other type? */
        assert(b_type == "int");

        decl_param_t *param = (decl_param_t *)aux;
        param->b_type = b_type;
        var_defs->Dump2StringIR(aux);
    }
};

class VarDefsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_var_defs;

    void Dump() const override {
        std::cout << " VarDefsAST { ";
        auto ii = vec_var_defs.begin();
        auto ie = vec_var_defs.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " } ";
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
    std::unique_ptr<BaseAST> const_exp_array;

    void Dump() const override {
        std::cout << " VarDefAST { ";
        std::cout << ident;
        if(const_exp_array != nullptr){
            const_exp_array->Dump();
        }
        if(init_val != nullptr){
            std::cout << " = ";
            init_val->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        decl_param_t *param = (decl_param_t *)aux;
        /* FURTHER: what if other type */
        assert((param->b_type) == "int");
        std::string string_koopa_type = "i32";
        bool is_global_var = param->is_global;

        if(const_exp_array == nullptr){
            if(init_val == nullptr){
                int cur_namespace = stack_namespace.top();
                assert(!symbol_tables[cur_namespace].bool_symbol_exist_local(ident));

                symbol_tables[cur_namespace].insert_var_definition_int(ident, stack_namespace.top());

                if(is_global_var){
                    std::cout << "global "
                    << symbol_tables[cur_namespace].get_var_pointer_int(ident)
                    << " = alloc "
                    << string_koopa_type
                    << ", zeroinit"
                    << std::endl;
                }
                else{
                    std::cout << "\t"
                    << symbol_tables[cur_namespace].get_var_pointer_int(ident)
                    << " = alloc "
                    << string_koopa_type
                    << std::endl;
                }
            }
            else{
                init_val_result_t init_val_result;
                init_val->Dump2StringIR(&init_val_result);

                if(is_global_var){
                    /*
                        TODO: what if not depth == 0?
                        e.g., int x = 1, y = x + 1; (?)
                    */
                    assert(!init_val_result.is_array);
                    assert(init_val_result.exp_result.is_zero_depth);

                    int cur_namespace = stack_namespace.top();
                    assert(!symbol_tables[cur_namespace].bool_symbol_exist_local(ident));

                    symbol_tables[cur_namespace].insert_var_definition_int(ident, stack_namespace.top());

                    std::cout << "global "
                    << symbol_tables[cur_namespace].get_var_pointer_int(ident)
                    << " = alloc "
                    << string_koopa_type
                    << ", "
                    << init_val_result.exp_result.result_number
                    << std::endl;
                }
                else{
                    std::string string_value_to_be_stored;
                    if(init_val_result.exp_result.is_zero_depth){
                        /* the value is in init_val_result.result_number */
                        string_value_to_be_stored = std::to_string(init_val_result.exp_result.result_number);
                    }
                    else{
                        /* the value is in %init_val_result.result_id*/
                        string_value_to_be_stored = "%" + std::to_string(init_val_result.exp_result.result_id);
                    }

                    int cur_namespace = stack_namespace.top();
                    assert(!symbol_tables[cur_namespace].bool_symbol_exist_local(ident));

                    symbol_tables[cur_namespace].insert_var_definition_int(ident, stack_namespace.top());

                    std::string string_var_pointer = symbol_tables[cur_namespace].get_var_pointer_int(ident);

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
        }
        else{
            if(init_val == nullptr){
                const_exp_array_result_t cear;
                const_exp_array->Dump2StringIR(&cear);
                int array_length = cear.array_length;

                int cur_namespace = stack_namespace.top();
                assert(!symbol_tables[cur_namespace].bool_symbol_exist_local(ident));

                symbol_tables[cur_namespace].insert_array_definition_int(ident, stack_namespace.top());

                if(is_global_var){
                    std::cout << "global" << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                    std::cout << " = alloc ";
                    std::cout << "[";
                    std::cout << "i32";
                    std::cout << ", " << array_length << "]";
                    std::cout << ", zeroinit" << std::endl;
                }
                else{
                    std::cout << "\t" << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                    std::cout << " = alloc ";
                    std::cout << "[";
                    std::cout << "i32";
                    std::cout << ", " << array_length << "]";
                }
            }
            else{
                init_val_result_t init_val_result;
                init_val->Dump2StringIR(&init_val_result);
                assert(init_val_result.is_array);

                const_exp_array_result_t cear;
                const_exp_array->Dump2StringIR(&cear);
                int array_length = cear.array_length;

                int cur_namespace = stack_namespace.top();
                assert(!symbol_tables[cur_namespace].bool_symbol_exist_local(ident));

                symbol_tables[cur_namespace].insert_array_definition_int(ident, stack_namespace.top());

                if(is_global_var){
                    std::cout << "global" << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                    std::cout << " = alloc ";
                    std::cout << "[";
                    std::cout << "i32";
                    std::cout << ", " << array_length << "]";
                    std::cout << ", ";
                    if(init_val_result.length == 0){
                        std::cout << "zeroinit" << std::endl;
                    }
                    else{
                        std::cout << "{";
                        for(int i = 0; i < array_length; ++i){
                            if(i){
                                std::cout << ", ";
                            }
                            if(i < init_val_result.length){
                                assert(init_val_result.exp_result_array[i].is_zero_depth);
                                std::cout << init_val_result.exp_result_array[i].result_number;
                            }
                            else{
                                std::cout << 0;
                            }
                        }
                        std::cout << "}" << std::endl;
                    }
                }
                else{
                    std::cout << "\t" << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                    std::cout << " = alloc ";
                    std::cout << "[";
                    std::cout << "i32";
                    std::cout << ", " << array_length << "]" << std::endl;

                    for(int i = 0; i < array_length; ++i){
                        std::cout << "\t%" << result_id++ << " = getelemptr ";
                        std::cout << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                        std::cout << ", " << i << std::endl;
                        if(i < init_val_result.length){
                            std::cout << "\tstore ";
                            if(init_val_result.exp_result_array[i].is_zero_depth){
                                std::cout << init_val_result.exp_result_array[i].result_number;
                            }
                            else{
                                std::cout << "%";
                                std::cout << init_val_result.exp_result_array[i].result_id;
                            }
                        }
                        else{
                            std::cout << "\tstore 0";
                        }
                        std::cout << ", %" << result_id - 1 << std::endl;
                    }
                }
            }
        }
    }
};

/* InitVal       ::= Exp; */
class InitValAST : public BaseAST {
public:
    int type;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> init_val_array;

    void Dump() const override {
        std::cout << " InitValAST { ";
        std::cout << " type: " << type << ", ";
        if(type == 0){
            exp->Dump();
        }
        else{
            std::cout << " { ";
            if(init_val_array != nullptr){
                init_val_array->Dump();
            }
            std::cout << " } ";
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        init_val_result_t *ivr = (init_val_result_t *)aux;
        if(type == 0){
            ivr->is_array = false;
            exp_result_t exp_result;
            exp->Dump2StringIR(&exp_result);
            ivr->exp_result = exp_result;
        }
        else{
            ivr->is_array = true;
            if(init_val_array == nullptr){
                ivr->length = 0;
            }
            else{
                init_val_array->Dump2StringIR(ivr);
            }
        }
    }
};

class InitValArrayAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_exps;

    void Dump() const override {
        std::cout << " InitValArrayAST { ";
        auto ii = vec_exps.begin();
        auto ie = vec_exps.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        init_val_result_t *ivr = (init_val_result_t *)aux;
        ivr->exp_result_array = std::vector<exp_result_t>();
        ivr->length = 0;
        auto ii = vec_exps.begin();
        auto ie = vec_exps.end();
        for(; ii != ie; ++ii){
            exp_result_t exp_result;
            (*ii)->Dump2StringIR(&exp_result);
            ivr->exp_result_array.push_back(exp_result);
            ivr->length++;
        }
    }
};

/* Part 3: Func */
/* FuncDef     ::= FuncType IDENT "(" [FuncFParams] ")" Block; */
class FuncDefAST : public BaseAST {
public:
    std::string func_type;
    std::string ident;
    std::unique_ptr<BaseAST> func_f_params;
    std::unique_ptr<BaseAST> block;

    void Dump() const override {
        std::cout << " FuncDefAST { ";
        std::cout << func_type << ", ";
        std::cout << ident << ", ";
        std::cout << " ( ";
        if(func_f_params != nullptr){
            func_f_params->Dump();
        }
        std::cout << " ) ";
        block->Dump();
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        /* TODO: params check? return type check? */

        std::cout << "fun " << "@" << ident << "(";

        int cur_namespace = stack_namespace.top();

        func_f_params_result_t func_f_params_result;
        if(func_f_params == nullptr){
            func_f_params_result.count = 0;
        }
        else{
            func_f_params->Dump2StringIR(&func_f_params_result);
        }

        std::cout << ")";

        assert(func_type == "int" || func_type == "void");
        if(func_type == "int"){
            std::cout << ": i32";
        }

        symbol_tables[cur_namespace].insert_func_def(ident, func_type);

        std::cout << " {" << std::endl;
        std::cout << "%" << "entry" << ":" << std::endl;

        block->Dump2StringIR(&func_f_params_result);

        std::cout << "\tret" << std::endl;
        std::cout << "}" << std::endl;
    }
};

/* FuncFParams ::= FuncFParam {"," FuncFParam}; */
class FuncFParamsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_func_f_params;

    void Dump() const override {
        std::cout << " FuncFParamsAST { ";
        auto ii = vec_func_f_params.begin();
        auto ie = vec_func_f_params.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " } ";
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
    std::string b_type;
    std::string ident;

    void Dump() const override {
        std::cout << " FuncFParamAST { ";
        std::cout << " BType:" << b_type << ", ";
        std::cout << ident;
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        func_f_param_result_t *param_r = (func_f_param_result_t *)aux;

        /* FURTHER: what if other types */
        assert(b_type == "int");
        std::cout << "@" << ident << ": " << "i32";

        param_r->b_type = b_type;
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
        std::cout << " BlockAST { ";
        if(block_items != nullptr){
            block_items->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        /* aux MIGHT be nullptr */
        func_f_params_result_t *params = (func_f_params_result_t *)aux;

        assert(!stack_namespace.empty());
        symbol_tables[id] = SymbolTable(stack_namespace.top());
        stack_namespace.push(id);

        if(params != nullptr){
            int count = params->count;
            for(int i = 0; i < count; ++i){
                std::string &ident = params->params[i].ident;
                assert(!symbol_tables[id].bool_symbol_exist_local(ident));

                /* FURTHER: what if other types */
                assert(params->params[i].b_type == "int");
                symbol_tables[id].insert_var_func_param_int(ident, id);

                std::cout << "\t" << symbol_tables[id].get_var_pointer_int(ident);
                std::cout << "= alloc i32" << std::endl;

                std::cout << "\tstore @" << ident << ", ";
                std::cout << symbol_tables[id].get_var_pointer_int(ident);
                std::cout << std::endl;
            }
        }

        if(block_items != nullptr){
            block_items->Dump2StringIR(nullptr);
        }

        stack_namespace.pop();
        symbol_tables[id].clear_table();
    }
};

class BlockItemsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_block_items;

    void Dump() const override {
        std::cout << " BlockItemsAST { ";
        auto ii = vec_block_items.begin();
        auto ie = vec_block_items.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux [[maybe_unused]]) const override {
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
    block_item_type_t type;
    std::unique_ptr<BaseAST> item;

    void Dump() const override {
        std::cout << " BlockItemAST { ";
        std::cout << " type: " << type << ", ";
        item->Dump();
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux [[maybe_unused]]) const override {
        decl_param_t param;
        switch (type)
        {
        case BLOCK_ITEM_DECL:
            param.is_global = false;
            item->Dump2StringIR(&param);
            break;
        case BLOCK_ITEM_GENERAL_STMT:
            item->Dump2StringIR(nullptr);
            break;
        default:
            assert(false);
            break;
        }
    }
};

/* GeneralStmt     ::= Stmt | OpenStmt; */
class GeneralStmtAST : public BaseAST {
public:
    general_stmt_type_t type;
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override {
        std::cout << " GeneralStmtAST { ";
        std::cout << " type: " << type << ", ";
        stmt->Dump();
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux [[maybe_unused]]) const override {
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
    stmt_type_t type;

    int ret_id;
    std::unique_ptr<BaseAST> l_val;
    std::unique_ptr<BaseAST> exp;

    std::unique_ptr<BaseAST> block;

    int if_stmt_id;
    std::unique_ptr<BaseAST> stmt_true;
    std::unique_ptr<BaseAST> stmt_false;

    int while_id;
    std::unique_ptr<BaseAST> stmt_body;

    void Dump() const override {
        std::cout << " StmtAST { ";
        std::cout << " type: " << type << ", ";
        switch (type)
        {
        case STMT_ASSIGN:
            l_val->Dump();
            std::cout << " = ";
            exp->Dump();
            std::cout << " ; ";
            break;
        case STMT_EXP:
            if(exp != nullptr){
                exp->Dump();
            }
            std::cout << " ; ";
            break;
        case STMT_BLOCK:
            block->Dump();
            break;
        case STMT_RETURN:
            std::cout << " return ";
            if(exp != nullptr){
                exp->Dump();
            }
            std::cout << " ; ";
            break;
        case STMT_IF_STMT:
            std::cout << " if ( ";
            exp->Dump();
            std::cout << " ) ";
            stmt_true->Dump();
            std::cout << " else ";
            stmt_false->Dump();
            break;
        case STMT_WHILE_STMT:
            std::cout << " while ( ";
            exp->Dump();
            std::cout << " ) ";
            stmt_body->Dump();
            std::cout << " } ";
            break;
        case STMT_BREAK:
            std::cout << " break ; ";
            break;
        case STMT_CONTINUE:
            std::cout << "continue ; ";
            break;
        default:
            assert(false);
            break;
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux [[maybe_unused]]) const override {
        exp_result_t exp_result;
        l_val_result_t l_val_result;
        int target;
        switch (type)
        {
        case STMT_ASSIGN:
            exp->Dump2StringIR(&exp_result);

            l_val_result.lhs = true;
            l_val->Dump2StringIR(&l_val_result);

            assert(l_val_result.is_var);
            if(exp_result.is_zero_depth){
                std::cout << "\tstore " << exp_result.result_number
                        << ", " << l_val_result.pointer << std::endl;
            }
            else{
                std::cout << "\tstore %" << exp_result.result_id
                        << ", " << l_val_result.pointer << std::endl;
            }
            break;
        case STMT_EXP:
            if(exp != nullptr){
                exp->Dump2StringIR(&exp_result);
            }
            break;
        case STMT_BLOCK:
            block->Dump2StringIR(nullptr);
            break;
        case STMT_RETURN:
            std::cout << "\tjump %ret_" << ret_id << std::endl;
            std::cout << "%ret_" << ret_id << ":" << std::endl;
            if(exp == nullptr){
                std::cout << "\tret" << std::endl;
            }
            else{
                exp->Dump2StringIR(&exp_result);

                if(exp_result.is_zero_depth){
                    std::cout << "\tret " << exp_result.result_number << std::endl;
                }
                else{
                    std::cout << "\tret %" << exp_result.result_id << std::endl;
                }
            }
            std::cout << "%after_ret_" << ret_id << ":" << std::endl;
            break;
        case STMT_IF_STMT:
            exp->Dump2StringIR(&exp_result);
            if(exp_result.is_zero_depth){
                std::cout << "\tbr " << exp_result.result_number << ", ";
            }
            else{
                std::cout << "\tbr %" << exp_result.result_id << ", ";
            }
            std::cout << "%then_" << if_stmt_id << ", ";
            std::cout << "%else_" << if_stmt_id << std::endl;

            std::cout << "%then_" << if_stmt_id << ":" << std::endl;
            stmt_true->Dump2StringIR(nullptr);
            std::cout << "\tjump %end_" << if_stmt_id << std::endl;

            std::cout << "%else_" << if_stmt_id << ":" << std::endl;
            stmt_false->Dump2StringIR(nullptr);
            std::cout << "\tjump %end_" << if_stmt_id << std::endl;

            std::cout << "%end_" << if_stmt_id << ":" << std::endl;
            break;
        case STMT_WHILE_STMT:
            stack_while_id.push(while_id);

            std::cout << "\tjump %while_cond_" << while_id << std::endl;

            std::cout << "%while_cond_" << while_id << ":" << std::endl;
            exp->Dump2StringIR(&exp_result);
            if(exp_result.is_zero_depth){
                std::cout << "\tbr " << exp_result.result_number << ", ";
            }
            else{
                std::cout << "\tbr %" << exp_result.result_id << ", ";
            }
            std::cout << "%while_body_" << while_id << ", ";
            std::cout << "%while_end_" << while_id << std::endl;

            std::cout << "%while_body_" << while_id << ":" << std::endl;
            stmt_body->Dump2StringIR(nullptr);
            std::cout << "\tjump %while_cond_" << while_id << std::endl;

            std::cout << "%while_end_" << while_id << ":" << std::endl;

            stack_while_id.pop();
            break;
        case STMT_BREAK:
            assert(!stack_while_id.empty());
            target = stack_while_id.top();

            std::cout << "\tjump %while_end_" << target << std::endl;
            std::cout << "%after_break_while_" << target << ":";
            std::cout << std::endl;
            break;
        case STMT_CONTINUE:
            assert(!stack_while_id.empty());
            target = stack_while_id.top();

            std::cout << "\tjump %while_cond_" << target << std::endl;
            std::cout << "%after_continue_while_" << target << ":";
            std::cout << std::endl;
            break;
        default:
            assert(false);
            break;
        }
    }
};

/*
    OpenStmt    ::= IF '(' Exp ')' GeneralStmt
                | IF '(' Exp ')' Stmt ELSE OpenStmt
*/
class OpenStmtAST : public BaseAST {
public:
    open_stmt_type_t type;
    int if_stmt_id;
    int while_id;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> stmt_true;
    std::unique_ptr<BaseAST> stmt_false;
    std::unique_ptr<BaseAST> stmt_body;

    void Dump() const override {
        std::cout << " OpenStmtAST { ";
        std::cout << " type: " << type << ", ";
        switch (type)
        {
        case OPEN_STMT_IF_GENERAL_STMT:
            std::cout << " if ( ";
            exp->Dump();
            std::cout << " ) ";
            stmt_true->Dump();
            break;
        case OPEN_STMT_IF_STMT_OPEN_STMT:
            std::cout << " if ( ";
            exp->Dump();
            std::cout << " ) ";
            stmt_true->Dump();
            std::cout << " else ";
            stmt_false->Dump();
            break;
        case OPEN_STMT_WHILE_OPEN_STMT:
            std::cout << "while ( ";
            exp->Dump();
            std::cout << " ) ";
            stmt_body->Dump();
            break;
        default:
            assert(false);
            break;
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux [[maybe_unused]]) const override {
        exp_result_t exp_result;
        switch (type)
        {
        case OPEN_STMT_IF_GENERAL_STMT:
            exp->Dump2StringIR(&exp_result);
            if(exp_result.is_zero_depth){
                std::cout << "\tbr " << exp_result.result_number << ", ";
            }
            else{
                std::cout << "\tbr %" << exp_result.result_id << ", ";
            }
            std::cout << "%then_" << if_stmt_id << ", ";
            std::cout << "%end_" << if_stmt_id << std::endl;

            std::cout << "%then_" << if_stmt_id << ":" << std::endl;
            stmt_true->Dump2StringIR(nullptr);
            std::cout << "\tjump %end_" << if_stmt_id << std::endl;

            std::cout << "%end_" << if_stmt_id << ":" << std::endl;
            break;
        case OPEN_STMT_IF_STMT_OPEN_STMT:
            exp->Dump2StringIR(&exp_result);
            if(exp_result.is_zero_depth){
                std::cout << "\tbr " << exp_result.result_number << ", ";
            }
            else{
                std::cout << "\tbr %" << exp_result.result_id << ", ";
            }
            std::cout << "%then_" << if_stmt_id << ", ";
            std::cout << "%else_" << if_stmt_id << std::endl;

            std::cout << "%then_" << if_stmt_id << ":" << std::endl;
            stmt_true->Dump2StringIR(nullptr);
            std::cout << "\tjump %end_" << if_stmt_id << std::endl;

            std::cout << "%else_" << if_stmt_id << ":" << std::endl;
            stmt_false->Dump2StringIR(nullptr);
            std::cout << "\tjump %end_" << if_stmt_id << std::endl;

            std::cout << "%end_" << if_stmt_id << ":" << std::endl;
            break;
        case OPEN_STMT_WHILE_OPEN_STMT:
            stack_while_id.push(while_id);

            std::cout << "\tjump %while_cond_" << while_id << std::endl;

            std::cout << "%while_cond_" << while_id << ":" << std::endl;
            exp->Dump2StringIR(&exp_result);
            if(exp_result.is_zero_depth){
                std::cout << "\tbr " << exp_result.result_number << ", ";
            }
            else{
                std::cout << "\tbr %" << exp_result.result_id << ", ";
            }
            std::cout << "%while_body_" << while_id << ", ";
            std::cout << "%while_end_" << while_id << std::endl;

            std::cout << "%while_body_" << while_id << ":" << std::endl;
            stmt_body->Dump2StringIR(nullptr);
            std::cout << "\tjump %while_cond_" << while_id << std::endl;

            std::cout << "%while_end_" << while_id << ":" << std::endl;

            stack_while_id.pop();
            break;
        default:
            assert(false);
            break;
        }
    }
};

/* Part 5: Exp */
/* Exp         ::= LOrExp; */
class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> l_or_exp;

    void Dump() const override {
        std::cout << " ExpAST { ";
        l_or_exp->Dump();
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        l_or_exp->Dump2StringIR(aux);
    }
};

/* LVal          ::= IDENT; */
class LValAST : public BaseAST {
public:
    std::string ident;
    std::unique_ptr<BaseAST> exp_array;

    void Dump() const override {
        std::cout << " LValAST { ";
        std::cout << ident;
        if(exp_array != nullptr){
            exp_array->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        l_val_result_t *lval = (l_val_result_t *)aux;
        int cur_namespace = stack_namespace.top();
        if(exp_array == nullptr){
            if(lval->lhs){
                assert(symbol_tables[cur_namespace].bool_symbol_is_var_int(ident));
                lval->is_var = true;
                lval->pointer =
                    symbol_tables[cur_namespace].get_var_pointer_int(ident);
            }
            else{
                bool is_var = symbol_tables[cur_namespace].bool_symbol_is_var_int(ident);
                lval->is_var = is_var;
                if(is_var){
                    lval->pointer =
                        symbol_tables[cur_namespace].get_var_pointer_int(ident);

                    std::cout << "\t%" << result_id++ << " = load "
                            << lval->pointer
                            << std::endl;
                }
                else{
                    lval->val =
                        symbol_tables[cur_namespace].get_const_definition_int(ident);
                }
            }
        }
        else{
            exp_array_result_t ear;
            exp_array->Dump2StringIR(&ear);

            lval->is_var = true;
            if(lval->lhs){
                /* Assign (lhs) */
                lval->pointer = "%" + std::to_string(result_id++);
                std::cout << "\t" << lval->pointer << " = getelemptr ";
                std::cout << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                if(ear.exp_idx.is_zero_depth){
                    std::cout << ", " << ear.exp_idx.result_number;
                }
                else{
                    std::cout << ", " << "%" << ear.exp_idx.result_id;
                }
                std::cout << std::endl;
            }
            else{
                /* Get Value (rhs) */
                lval->pointer = "%" + std::to_string(result_id++);
                std::cout << "\t" << lval->pointer << " = getelemptr ";
                std::cout << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                if(ear.exp_idx.is_zero_depth){
                    std::cout << ", " << ear.exp_idx.result_number;
                }
                else{
                    std::cout << ", " << "%" << ear.exp_idx.result_id;
                }
                std::cout << std::endl;

                std::cout << "\t%" << result_id++ << " = load ";
                std::cout << lval->pointer << std::endl;
            }
        }
    }
};

class ExpArrayAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << " ExpArrayAST { ";
        std::cout << "[";
        exp->Dump();
        std::cout << "]";
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        exp_array_result_t *ear = (exp_array_result_t *)aux;
        exp_result_t exp_result;
        exp->Dump2StringIR(&exp_result);
        ear->exp_idx = exp_result;
    }
};

/*
    PrimaryExp    ::= "(" Exp ")" | LVal | Number;
    Number      ::= INT_CONST;
*/
class PrimaryExpAST : public BaseAST {
public:
    primary_exp_type_t type;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> l_val;
    int number;

    void Dump() const override {
        std::cout << " PrimaryExpAST { ";
        std::cout << " type: " << type << ", ";
        switch (type)
        {
        case PRIMARY_EXP_EXP:
            std::cout << " ( ";
            exp->Dump();
            std::cout << " ) ";
            break;
        case PRIMARY_EXP_L_VAL:
            l_val->Dump();
            break;
        case PRIMARY_EXP_NUMBER:
            std::cout << number;
            break;
        default:
            assert(false);
            break;
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        l_val_result_t l_val_result;
        switch (type)
        {
        case PRIMARY_EXP_EXP:
            exp->Dump2StringIR(aux);
            break;
        case PRIMARY_EXP_L_VAL:
            l_val_result.lhs = false;
            l_val->Dump2StringIR(&l_val_result);

            if(l_val_result.is_var){
                ((exp_result_t *)aux)->is_zero_depth = false;
                ((exp_result_t *)aux)->result_id = result_id - 1;
            }
            else{
                ((exp_result_t *)aux)->is_zero_depth = true;
                ((exp_result_t *)aux)->result_number = l_val_result.val;
            }
            break;
        case PRIMARY_EXP_NUMBER:
            ((exp_result_t *)aux)->is_zero_depth = true;
            ((exp_result_t *)aux)->result_number = number;
            break;
        default:
            assert(false);
            break;
        }
    }
};

/*
    UnaryExp    ::= PrimaryExp | IDENT "(" [FuncRParams] ")" | UnaryOp UnaryExp;
    UnaryOp     ::= "+" | "-" | "!";
*/
class UnaryExpAST : public BaseAST {
public:
    unary_exp_type_t type;
    std::unique_ptr<BaseAST> primary_exp;
    std::string unary_op;
    std::unique_ptr<BaseAST> unary_exp;
    std::string ident;
    std::unique_ptr<BaseAST> func_r_params;

    void Dump() const override {
        std::cout << " UnaryExpAST { ";
        std::cout << " type: " << type << ", ";
        switch (type)
        {
        case UNARY_EXP_PRIMARY_EXP:
            primary_exp->Dump();
            break;
        case UNARY_EXP_UNARY_OP_EXP:
            std::cout << unary_op;
            unary_exp->Dump();
            break;
        case UNARY_EXP_FUNCTION_CALL:
            std::cout << ident << " ( ";
            if(func_r_params != nullptr){
                func_r_params->Dump();
            }
            std::cout << " ) ";
            break;
        default:
            assert(false);
            break;
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        exp_result_t *result = (exp_result_t *)aux;
        exp_result_t result1;
        func_r_params_result_t params;
        int cur_namespace;
        std::string type_return;
        int i, count;
        switch (type)
        {
        case UNARY_EXP_PRIMARY_EXP:
            primary_exp->Dump2StringIR(aux);
            break;
        case UNARY_EXP_UNARY_OP_EXP:
            assert(unary_op == "+" || unary_op == "-" || unary_op == "!");
            unary_exp->Dump2StringIR(&result1);
            if(result1.is_zero_depth){
                result->is_zero_depth = true;
                switch (unary_op[0])
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
                switch (unary_op[0])
                {
                case '+':
                    /* do nothing */
                    result->is_zero_depth = result1.is_zero_depth;
                    result->result_id = result_id;
                    break;
                case '-':
                    result->is_zero_depth = false;
                    result->result_id = result_id++;
                    std::cout << "\t%" << result->result_id << " = sub 0, ";
                    std::cout << "%" << result1.result_id << std::endl;
                    break;
                case '!':
                    result->is_zero_depth = false;
                    result->result_id = result_id++;
                    std::cout << "\t%" << result->result_id << " = eq 0, ";
                    std::cout << "%" << result1.result_id << std::endl;
                    break;
                default:
                    assert(false);
                    break;
                }
            }
            break;
        case UNARY_EXP_FUNCTION_CALL:
            if(func_r_params != nullptr){
                func_r_params->Dump2StringIR(&params);
            }

            result->is_zero_depth = false;

            cur_namespace = stack_namespace.top();

            type_return = symbol_tables[cur_namespace].get_func_return_type(ident);
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
                i = 0;
                count = params.count;
                if(i != count){
                    if(params.params[i].is_zero_depth){
                        std::cout << params.params[i].result_number;
                    }
                    else{
                        std::cout << "%" << params.params[i].result_id;
                    }
                    ++i;
                }
                for(; i < params.count; ++i){
                    std::cout << ", ";
                    if(params.params[i].is_zero_depth){
                        std::cout << params.params[i].result_number;
                    }
                    else{
                        std::cout << "%" << params.params[i].result_id;
                    }
                }
            }

            std::cout << ")" << std::endl;
            break;
        default:
            assert(false);
            break;
        }
    }
};

/* FuncRParams ::= Exp {"," Exp}; */
class FuncRParamsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_func_r_params;

    void Dump() const override {
        std::cout << " FuncRParamsAST { ";
        auto ii = vec_func_r_params.begin();
        auto ie = vec_func_r_params.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        func_r_params_result_t *params = (func_r_params_result_t *)aux;
        params->count = 0;
        params->params = std::vector<exp_result_t>();

        auto ii = vec_func_r_params.begin();
        auto ie = vec_func_r_params.end();
        for(; ii != ie; ++ii){
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
    mul_exp_type_t type;
    std::unique_ptr<BaseAST> unary_exp;
    std::unique_ptr<BaseAST> mul_exp;
    std::string op;

    void Dump() const override {
        std::cout << " MulExpAST { ";
        std::cout << " type: " << type << ", ";
        if(type == MUL_EXP_UNARY){
            unary_exp->Dump();
        }
        else{
            mul_exp->Dump();
            std::cout << " " << op << " ";
            unary_exp->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        if(type == MUL_EXP_UNARY){
            unary_exp->Dump2StringIR(aux);
        }
        else{
            assert(op == "%" || op == "*" || op == "/");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            mul_exp->Dump2StringIR(&result1);
            unary_exp->Dump2StringIR(&result2);

            if(result1.is_zero_depth && result2.is_zero_depth){
                result->is_zero_depth = true;
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
                result->is_zero_depth = false;
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

                if(result1.is_zero_depth){
                    std::cout << result1.result_number;
                }
                else{
                    std::cout << "%" << result1.result_id;
                }
                std::cout << ", ";
                if(result2.is_zero_depth){
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
    add_exp_type_t type;
    std::unique_ptr<BaseAST> mul_exp;
    std::unique_ptr<BaseAST> add_exp;
    std::string op;

    void Dump() const override {
        std::cout << " AddExpAST { ";
        std::cout << " type: " << type << ", ";
        if(type == ADD_EXP_MUL){
            mul_exp->Dump();
        }
        else{
            add_exp->Dump();
            std::cout << " " << op << " ";
            mul_exp->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        if(type == ADD_EXP_MUL){
            mul_exp->Dump2StringIR(aux);
        }
        else{
            assert(op == "+" || op == "-");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            add_exp->Dump2StringIR(&result1);
            mul_exp->Dump2StringIR(&result2);

            if(result1.is_zero_depth && result2.is_zero_depth){
                result->is_zero_depth = true;
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
                result->is_zero_depth = false;
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

                if(result1.is_zero_depth){
                    std::cout << result1.result_number;
                }
                else{
                    std::cout << "%" << result1.result_id;
                }
                std::cout << ", ";
                if(result2.is_zero_depth){
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
    rel_exp_type_t type;
    std::unique_ptr<BaseAST> add_exp;
    std::unique_ptr<BaseAST> rel_exp;
    std::string op;

    void Dump() const override {
        std::cout << " RelExpAST { ";
        std::cout << " type: " << type << ", ";
        if(type == REL_EXP_ADD){
            add_exp->Dump();
        }
        else{
            rel_exp->Dump();
            std::cout << " " << op << " ";
            add_exp->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        if(type == REL_EXP_ADD){
            add_exp->Dump2StringIR(aux);
        }
        else{
            assert(op == "<" || op == ">" || op == "<=" || op == ">=");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            rel_exp->Dump2StringIR(&result1);
            add_exp->Dump2StringIR(&result2);

            if(result1.is_zero_depth && result2.is_zero_depth){
                result->is_zero_depth = true;
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
                result->is_zero_depth = false;
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

                if(result1.is_zero_depth){
                    std::cout << result1.result_number;
                }
                else{
                    std::cout << "%" << result1.result_id;
                }
                std::cout << ", ";
                if(result2.is_zero_depth){
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
    eq_exp_type_t type;
    std::unique_ptr<BaseAST> rel_exp;
    std::unique_ptr<BaseAST> eq_exp;
    std::string op;

    void Dump() const override {
        std::cout << " EqExpAST { ";
        std::cout << " type: " << type << ", ";
        if(type == EQ_EXP_REL){
            rel_exp->Dump();
        }
        else{
            eq_exp->Dump();
            std::cout << " " << op << " ";
            rel_exp->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        if(type == EQ_EXP_REL){
            rel_exp->Dump2StringIR(aux);
        }
        else{
            assert(op == "==" || op == "!=");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            rel_exp->Dump2StringIR(&result1);
            eq_exp->Dump2StringIR(&result2);

            if(result1.is_zero_depth && result2.is_zero_depth){
                result->is_zero_depth = true;
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
                result->is_zero_depth = false;
                result->result_id = result_id++;

                if(op == "=="){
                    std::cout << "\t%" << result->result_id << " = eq ";
                }
                else if(op == "!="){
                    std::cout << "\t%" << result->result_id << " = ne ";
                }

                if(result1.is_zero_depth){
                    std::cout << result1.result_number;
                }
                else{
                    std::cout << "%" << result1.result_id;
                }
                std::cout << ", ";
                if(result2.is_zero_depth){
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
    l_and_exp_type_t type;
    int id;
    std::unique_ptr<BaseAST> eq_exp;
    std::unique_ptr<BaseAST> l_and_exp;
    std::string op;

    void Dump() const override {
        std::cout << " LAndExpAST { ";
        std::cout << " type: " << type << ", ";
        if(type == L_AND_EXP_EQ){
            eq_exp->Dump();
        }
        else{
            l_and_exp->Dump();
            std::cout << " " << op << " ";
            eq_exp->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        if(type == L_AND_EXP_EQ){
            eq_exp->Dump2StringIR(aux);
        }
        else{
            assert(op == "&&");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            l_and_exp->Dump2StringIR(&result1);

            /* short-circuit logic */
            if(result1.is_zero_depth){
                if(result1.result_number == 0){
                    result->is_zero_depth = true;
                    result->result_number = 0;
                }
                else{
                    eq_exp->Dump2StringIR(&result2);
                    if(result2.is_zero_depth){
                        result->is_zero_depth = true;
                        result->result_number = result1.result_number
                                            && result2.result_number;
                    }
                    else{
                        result->is_zero_depth = false;
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

                eq_exp->Dump2StringIR(&result2);

                std::cout << "\t%" << result_id++ << " = ne 0, ";
                std::cout << "%" << result1.result_id << std::endl;

                std::cout << "\t%" << result_id++ << " = ne 0, ";
                if(result2.is_zero_depth){
                    std::cout << result2.result_number;
                }
                else{
                    std::cout << "%" << result2.result_id;
                }
                std::cout << std::endl;

                result->is_zero_depth = false;
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
        }
    }
};

/* LOrExp      ::= LAndExp | LOrExp "||" LAndExp; */
class LOrExpAST : public BaseAST {
public:
    l_or_exp_type_t type;
    int id;
    std::unique_ptr<BaseAST> l_and_exp;
    std::unique_ptr<BaseAST> l_or_exp;
    std::string op;

    void Dump() const override {
        std::cout << " LOrExpAST { ";
        std::cout << " type: " << type << ", ";
        if(type == L_OR_EXP_L_AND){
            l_and_exp->Dump();
        }
        else{
            l_or_exp->Dump();
            std::cout << " " << op << " ";
            l_and_exp->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        if(type == L_OR_EXP_L_AND){
            l_and_exp->Dump2StringIR(aux);
        }
        else{
            assert(op == "||");
            exp_result_t result1, result2;
            exp_result_t *result = (exp_result_t *)aux;

            l_or_exp->Dump2StringIR(&result1);

            /* short-circuit logic */
            if(result1.is_zero_depth){
                if(result1.result_number != 0){
                    result->is_zero_depth = true;
                    result->result_number = 1;
                }
                else{
                    l_and_exp->Dump2StringIR(&result2);
                    if(result2.is_zero_depth){
                        result->is_zero_depth = true;
                        result->result_number = result1.result_number
                                            || result2.result_number;
                    }
                    else{
                        result->is_zero_depth = false;
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

                l_and_exp->Dump2StringIR(&result2);

                std::cout << "\t%" << result_id++ << " = or ";
                std::cout << "%" << result1.result_id << ", ";
                if(result2.is_zero_depth){
                    std::cout << result2.result_number;
                }
                else{
                    std::cout << "%" << result2.result_id;
                }
                std::cout << std::endl;

                result->is_zero_depth = false;
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
        }
    }
};

/* ConstExp      ::= Exp; */
class ConstExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << " ConstExpAST {";
        exp->Dump();
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        exp->Dump2StringIR(aux);
    }
};

#endif /**< src/ast.h */
