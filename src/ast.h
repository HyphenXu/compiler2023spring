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
    bool is_const;
    int val;
    std::string pointer;
} l_val_result_t;

typedef struct{
    int count;
    std::vector<exp_result_t> params;
} func_r_params_result_t;

typedef struct{
    bool is_global;
    std::string b_type;
} decl_param_t;

typedef struct{
    int dim;
    std::vector<int> array_size;
} const_exps_result_t;

// typedef struct{
//     bool is_array;

//     int val;

//     int length;
//     std::vector<int> init_val;
// } const_init_val_result_t;

typedef struct{
    std::string ident;

    bool is_array;

    int const_exp_result_number;

    int idx;
    int level;  /* zeroinit is available only when level = -1 */
    const_exps_result_t shape;
    bool is_global;
} const_init_val_param_t;

typedef struct{
    int type;
    std::string ident;
    std::string b_type;

    const_exps_result_t shape;
} func_f_param_result_t;

typedef struct{
    int count;
    std::vector<func_f_param_result_t> params;
} func_f_params_result_t;

// typedef struct{
//     bool is_array;

//     exp_result_t exp_result;

//     int length;
//     std::vector<exp_result_t> exp_result_array;
// } init_val_result_t;

typedef struct{
    std::string ident;

    bool is_array;

    exp_result_t exp_result;

    int idx;
    int level;
    const_exps_result_t shape;
    bool is_global;
} init_val_param_t;

typedef struct{
    int dim;
    std::vector<exp_result_t> idx;
} exps_result_t;

static std::string gen_getelemptr_const_exp_koopa_code(
        const std::string &pointer_array,
        int idx, const_exps_result_t &shape){

    int dim = shape.dim;
    std::vector<int> array_idx = std::vector<int>(dim);

    for(int i = dim - 1; i >= 0; --i){
        array_idx[i] = idx % shape.array_size[i];
        idx = idx / shape.array_size[i];
    }

    std::string pointer_lhs, pointer_rhs;
    pointer_rhs = pointer_array;
    for(int i = 0; i < dim; ++i){
        pointer_lhs = "%" + std::to_string(result_id++);
        std::cout << "\t" << pointer_lhs << " = getelemptr ";
        std::cout << pointer_rhs << ", " << array_idx[i] << std::endl;
        pointer_rhs = pointer_lhs;
    }
    return pointer_lhs;
}

static void gen_init_val_brackets_before(int idx, const_exps_result_t &shape){
    int dim = shape.dim;
    int alignment = 1;
    for(int i = dim - 1; i >= 0; --i){
        alignment *= shape.array_size[i];
        if(idx % alignment == 0){
            std::cout << "{";
        }
        else{
            break;
        }
    }
}

static void gen_init_val_brackets_after(int idx, const_exps_result_t &shape){
    int dim = shape.dim;
    int alignment = 1;
    for(int i = dim - 1; i >= 0; --i){
        alignment *= shape.array_size[i];
        if((idx + 1) % alignment == 0){
            std::cout << "}";
        }
        else{
            break;
        }
    }
}

// typedef struct{
//     exp_result_t exp_idx;
// } exps_result_t;

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
class ConstInitValsAST;
class ConstExpsAST;
class VarDeclAST;
class VarDefsAST;
class VarDefAST;        /*TODO*/
class InitValAST;       /*TODO*/
class InitValsAST;

/* Part 3: Func */
class FuncDefAST;
class FuncFParamsAST;
class FuncFParamAST;    /* TODO */

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
class ExpsAST;
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
    std::unique_ptr<BaseAST> const_exps;
    std::unique_ptr<BaseAST> const_init_val;

    void Dump() const override {
        std::cout << " ConstDefAST { ";
        std::cout << ident;
        if(const_exps != nullptr){
            const_exps->Dump();
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

        if(const_exps == nullptr){
            const_init_val_param_t civp;
            civp.ident = ident;
            civp.is_array = false;
            const_init_val->Dump2StringIR(&civp);
            symbol_tables[cur_namespace].insert_const_definition_int(
                ident, civp.const_exp_result_number
            );
        }
        else{
            const_init_val_param_t civp;
            civp.ident = ident;
            civp.is_array = true;

            civp.idx = 0;
            civp.level = -1;

            const_exps->Dump2StringIR(&civp.shape);
            int dim = civp.shape.dim;

            symbol_tables[cur_namespace].insert_array_definition_int(ident, stack_namespace.top(), dim);

            civp.is_global = param->is_global;
            if(param->is_global){
                std::cout << "global ";
                std::cout << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                std::cout << " = alloc ";
                for(int i = 0; i < dim; ++i){
                    std::cout << "[";
                }
                std::cout << "i32";
                for(int i = 0; i < dim; ++i){
                    std::cout << ", "
                    << civp.shape.array_size[dim - i - 1]
                    << "]";
                }
                std::cout << ", ";
                const_init_val->Dump2StringIR(&civp);
                std::cout << std::endl;
            }
            else{
                std::cout << "\t";
                std::cout << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                std::cout << " = alloc ";
                for(int i = 0; i < dim; ++i){
                    std::cout << "[";
                }
                std::cout << "i32";
                for(int i = 0; i < dim; ++i){
                    std::cout << ", "
                    << civp.shape.array_size[dim - i - 1]
                    << "]";
                }
                std::cout << std::endl;
                const_init_val->Dump2StringIR(&civp);
            }
        }
    }
};

/* ConstInitVal  ::= ConstExp; */
class ConstInitValAST : public BaseAST {
public:
    int type;
    std::unique_ptr<BaseAST> const_exp;
    std::unique_ptr<BaseAST> const_init_vals;

    void Dump() const override {
        std::cout << " ConstInitValAST { ";
        std::cout << " type: " << type << ", ";
        if(type == 0){
            const_exp->Dump();
        }
        else{
            std::cout << " { ";
            if(const_init_vals != nullptr){
                const_init_vals->Dump();
            }
            std::cout << " } ";
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        const_init_val_param_t *civp = (const_init_val_param_t *)aux;
        if(!civp->is_array){
            assert(type == 0);
            exp_result_t const_exp_result;
            const_exp->Dump2StringIR(&const_exp_result);
            assert(const_exp_result.is_zero_depth);
            civp->const_exp_result_number = const_exp_result.result_number;
        }
        else{
            if(civp->is_global){
                if(type == 0){
                    exp_result_t const_exp_result;
                    const_exp->Dump2StringIR(&const_exp_result);
                    assert(const_exp_result.is_zero_depth);
                    gen_init_val_brackets_before(civp->idx, civp->shape);
                    std::cout << const_exp_result.result_number;
                    gen_init_val_brackets_after(civp->idx, civp->shape);
                    ++civp->idx;
                }
                else{
                    if(civp->level == -1 && const_init_vals == nullptr){
                        /* zeroinit */
                        std::cout << "zeroinit";
                    }
                    else{
                        /* dims: [level, shape.dim-1] */
                        int old_level = civp->level;
                        int cur_dim = civp->shape.dim - 1;

                        /* assert align */
                        assert(civp->idx % civp->shape.array_size[cur_dim] == 0);

                        /* determine new level and alignment */
                        int alignment = civp->shape.array_size[cur_dim];
                        for(--cur_dim; cur_dim > old_level; --cur_dim){
                            if(civp->idx % (alignment * civp->shape.array_size[cur_dim]) != 0){
                                break;
                            }
                            alignment *= civp->shape.array_size[cur_dim];
                        }
                        civp->level = ++cur_dim;

                        // std::cout << "{";
                        if(const_init_vals != nullptr){
                            const_init_vals->Dump2StringIR(aux);
                        }
                        else{
                            gen_init_val_brackets_before(civp->idx, civp->shape);
                            std::cout << "0";
                            gen_init_val_brackets_after(civp->idx, civp->shape);
                            ++civp->idx;
                        }
                        while(civp->idx % alignment != 0){
                            std::cout << ", ";
                            gen_init_val_brackets_before(civp->idx, civp->shape);
                            std::cout << 0;
                            gen_init_val_brackets_after(civp->idx, civp->shape);
                            ++civp->idx;
                        }
                        // std::cout << "}";

                        /* restore level */
                        civp->level = old_level;
                    }
                }
            }
            else{
                if(type == 0){
                    exp_result_t const_exp_result;
                    const_exp->Dump2StringIR(&const_exp_result);
                    assert(const_exp_result.is_zero_depth);

                    std::string result_pointer = gen_getelemptr_const_exp_koopa_code(
                        symbol_tables[stack_namespace.top()].get_array_pointer_int(civp->ident),
                        civp->idx++, civp->shape
                    );
                    std::cout << "\tstore " << const_exp_result.result_number;
                    std::cout << ", " << result_pointer << std::endl;
                }
                else{
                    if(civp->level == -1 && const_init_vals == nullptr){
                        /* zeroinit */
                        int size_tot = 1;
                        for(int i = 0; i < civp->shape.dim; ++i){
                            size_tot *= civp->shape.array_size[i];
                        }

                        for(int i = 0; i < size_tot; ++i){
                            std::string result_pointer = gen_getelemptr_const_exp_koopa_code(
                                symbol_tables[stack_namespace.top()].get_array_pointer_int(civp->ident),
                                i, civp->shape
                            );
                            std::cout << "\tstore 0";
                            std::cout << ", " << result_pointer << std::endl;
                        }
                    }
                    else{
                        /* dims: [level, shape.dim-1] */
                        int old_level = civp->level;
                        int cur_dim = civp->shape.dim - 1;

                        /* assert align */
                        assert(civp->idx % civp->shape.array_size[cur_dim] == 0);

                        /* determine new level and alignment */
                        int alignment = civp->shape.array_size[cur_dim];
                        for(--cur_dim; cur_dim > old_level; --cur_dim){
                            if(civp->idx % (alignment * civp->shape.array_size[cur_dim]) != 0){
                                break;
                            }
                            alignment *= civp->shape.array_size[cur_dim];
                        }
                        civp->level = ++cur_dim;

                        if(const_init_vals != nullptr){
                            const_init_vals->Dump2StringIR(aux);
                        }
                        else{
                            std::string result_pointer = gen_getelemptr_const_exp_koopa_code(
                                symbol_tables[stack_namespace.top()].get_array_pointer_int(civp->ident),
                                civp->idx++, civp->shape
                            );
                            std::cout << "\tstore 0";
                            std::cout << ", " << result_pointer << std::endl;
                        }

                        while(civp->idx % alignment != 0){
                            std::string result_pointer = gen_getelemptr_const_exp_koopa_code(
                                symbol_tables[stack_namespace.top()].get_array_pointer_int(civp->ident),
                                civp->idx++, civp->shape
                            );
                            std::cout << "\tstore 0";
                            std::cout << ", " << result_pointer << std::endl;
                        }

                        /* restore level */
                        civp->level = old_level;
                    }
                }
            }
        }
    }
};

class ConstInitValsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_const_init_vals;

    void Dump() const override {
        std::cout << " ConstInitValsAST { ";
        auto ii = vec_const_init_vals.begin();
        auto ie = vec_const_init_vals.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        const_init_val_param_t *civp = (const_init_val_param_t *)aux;
        auto ii = vec_const_init_vals.begin();
        auto ie = vec_const_init_vals.end();
        if(ii != ie){
            (*ii)->Dump2StringIR(aux);
            ++ii;
        }
        for(; ii != ie; ++ii){
            if(civp->is_global){
                std::cout << ", ";
            }
            (*ii)->Dump2StringIR(aux);
        }
    }
};

class ConstExpsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_const_exps;

    void Dump() const override {
        std::cout << " ConstExpsAST { ";
        auto ii = vec_const_exps.begin();
        auto ie = vec_const_exps.end();
        for(; ii != ie; ++ii){
            std::cout << " [ ";
            (*ii)->Dump();
            std::cout << " ] ";
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        const_exps_result_t *cer = (const_exps_result_t *)aux;
        cer->dim = 0;
        cer->array_size = std::vector<int>();
        auto ii = vec_const_exps.begin();
        auto ie = vec_const_exps.end();
        for(; ii != ie; ++ii){
            exp_result_t const_exp_result;
            (*ii)->Dump2StringIR(&const_exp_result);
            assert(const_exp_result.is_zero_depth);
            cer->array_size.push_back(const_exp_result.result_number);
            ++cer->dim;
        }
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
    std::unique_ptr<BaseAST> const_exps;

    void Dump() const override {
        std::cout << " VarDefAST { ";
        std::cout << ident;
        if(const_exps != nullptr){
            const_exps->Dump();
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
        std::string base_type = "i32";

        bool is_global_var = param->is_global;

        if(const_exps == nullptr){
            if(init_val == nullptr){
                int cur_namespace = stack_namespace.top();
                assert(!symbol_tables[cur_namespace].bool_symbol_exist_local(ident));

                symbol_tables[cur_namespace].insert_var_definition_int(ident, stack_namespace.top());

                if(is_global_var){
                    std::cout << "global "
                    << symbol_tables[cur_namespace].get_var_pointer_int(ident)
                    << " = alloc "
                    << base_type
                    << ", zeroinit"
                    << std::endl;
                }
                else{
                    std::cout << "\t"
                    << symbol_tables[cur_namespace].get_var_pointer_int(ident)
                    << " = alloc "
                    << base_type
                    << std::endl;
                }
            }
            else{
                init_val_param_t ivp;
                ivp.ident = ident;
                ivp.is_array = false;
                init_val->Dump2StringIR(&ivp);

                if(param->is_global){
                    /*
                        TODO: what if not depth == 0?
                        e.g., int x = 1, y = x + 1; (?)
                    */
                    assert(ivp.exp_result.is_zero_depth);

                    int cur_namespace = stack_namespace.top();
                    assert(!symbol_tables[cur_namespace].bool_symbol_exist_local(ident));
                    symbol_tables[cur_namespace].insert_var_definition_int(ident, stack_namespace.top());

                    std::cout << "global "
                    << symbol_tables[cur_namespace].get_var_pointer_int(ident)
                    << " = alloc "
                    << base_type
                    << ", "
                    << ivp.exp_result.result_number
                    << std::endl;
                }
                else{
                    std::string string_value_to_be_stored;
                    if(ivp.exp_result.is_zero_depth){
                        string_value_to_be_stored = std::to_string(ivp.exp_result.result_number);
                    }
                    else{
                        string_value_to_be_stored = "%" + std::to_string(ivp.exp_result.result_id);
                    }

                    int cur_namespace = stack_namespace.top();
                    assert(!symbol_tables[cur_namespace].bool_symbol_exist_local(ident));
                    symbol_tables[cur_namespace].insert_var_definition_int(ident, stack_namespace.top());

                    std::string string_var_pointer = symbol_tables[cur_namespace].get_var_pointer_int(ident);

                    std::cout << "\t"
                        << string_var_pointer
                        << " = alloc "
                        << base_type
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
                int cur_namespace = stack_namespace.top();
                assert(!symbol_tables[cur_namespace].bool_symbol_exist_local(ident));

                init_val_param_t ivp;
                ivp.ident = ident;
                ivp.is_array = true;

                ivp.idx = 0;
                ivp.level = -1;

                const_exps->Dump2StringIR(&ivp.shape);
                int dim = ivp.shape.dim;

                symbol_tables[cur_namespace].insert_array_definition_int(ident, stack_namespace.top(), dim);

                ivp.is_global = param->is_global;

                if(param->is_global){
                    std::cout << "global ";
                    std::cout << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                    std::cout << " = alloc ";
                    for(int i = 0; i < dim; ++i){
                        std::cout << "[";
                    }
                    std::cout << base_type;
                    for(int i = 0; i < dim; ++i){
                        std::cout << ", "
                        << ivp.shape.array_size[dim - i - 1]
                        << "]";
                    }
                    std::cout << ", zeroinit";
                    std::cout << std::endl;
                }
                else{
                    std::cout << "\t";
                    std::cout << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                    std::cout << " = alloc ";
                    for(int i = 0; i < dim; ++i){
                        std::cout << "[";
                    }
                    std::cout << base_type;
                    for(int i = 0; i < dim; ++i){
                        std::cout << ", "
                        << ivp.shape.array_size[dim - i - 1]
                        << "]";
                    }
                    std::cout << std::endl;
                }
            }
            else{
                int cur_namespace = stack_namespace.top();
                assert(!symbol_tables[cur_namespace].bool_symbol_exist_local(ident));

                init_val_param_t ivp;
                ivp.ident = ident;
                ivp.is_array = true;

                ivp.idx = 0;
                ivp.level = -1;

                const_exps->Dump2StringIR(&ivp.shape);
                int dim = ivp.shape.dim;

                symbol_tables[cur_namespace].insert_array_definition_int(ident, stack_namespace.top(), dim);

                ivp.is_global = param->is_global;

                if(param->is_global){
                    std::cout << "global ";
                    std::cout << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                    std::cout << " = alloc ";
                    for(int i = 0; i < dim; ++i){
                        std::cout << "[";
                    }
                    std::cout << base_type;
                    for(int i = 0; i < dim; ++i){
                        std::cout << ", "
                        << ivp.shape.array_size[dim - i - 1]
                        << "]";
                    }
                    std::cout << ", ";
                    init_val->Dump2StringIR(&ivp);
                    std::cout << std::endl;
                }
                else{
                    std::cout << "\t";
                    std::cout << symbol_tables[cur_namespace].get_array_pointer_int(ident);
                    std::cout << " = alloc ";
                    for(int i = 0; i < dim; ++i){
                        std::cout << "[";
                    }
                    std::cout << base_type;
                    for(int i = 0; i < dim; ++i){
                        std::cout << ", "
                        << ivp.shape.array_size[dim - i - 1]
                        << "]";
                    }
                    std::cout << std::endl;
                    init_val->Dump2StringIR(&ivp);
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
    std::unique_ptr<BaseAST> init_vals;

    void Dump() const override {
        std::cout << " InitValAST { ";
        std::cout << " type: " << type << ", ";
        if(type == 0){
            exp->Dump();
        }
        else{
            std::cout << " { ";
            if(init_vals != nullptr){
                init_vals->Dump();
            }
            std::cout << " } ";
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        init_val_param_t *ivp = (init_val_param_t *)aux;
        if(!ivp->is_array){
            assert(type == 0);
            exp_result_t exp_result;
            exp->Dump2StringIR(&exp_result);
            ivp->exp_result = exp_result;
        }
        else{
            if(ivp->is_global){
                if(type == 0){
                    exp_result_t exp_result;
                    exp->Dump2StringIR(&exp_result);
                    assert(exp_result.is_zero_depth);
                    gen_init_val_brackets_before(ivp->idx, ivp->shape);
                    std::cout << exp_result.result_number;
                    gen_init_val_brackets_after(ivp->idx, ivp->shape);
                    ++ivp->idx;
                }
                else{
                    if(ivp->level == -1 && init_vals == nullptr){
                        std::cout << "zeroinit";
                    }
                    else{
                        int old_level = ivp->level;
                        int cur_dim = ivp->shape.dim - 1;

                        assert(ivp->idx % ivp->shape.array_size[cur_dim] == 0);

                        int alignment = ivp->shape.array_size[cur_dim];
                        for(--cur_dim; cur_dim > old_level; --cur_dim){
                            if(ivp->idx % (alignment * ivp->shape.array_size[cur_dim]) != 0){
                                break;
                            }
                            alignment *= ivp->shape.array_size[cur_dim];
                        }
                        ivp->level = ++cur_dim;

                        // std::cout << "{";
                        if(init_vals != nullptr){
                            init_vals->Dump2StringIR(aux);
                        }
                        else{
                            gen_init_val_brackets_before(ivp->idx, ivp->shape);
                            std::cout << "0";
                            gen_init_val_brackets_after(ivp->idx, ivp->shape);
                            ++ivp->idx;
                        }
                        while(ivp->idx % alignment != 0){
                            std::cout << ", ";
                            gen_init_val_brackets_before(ivp->idx, ivp->shape);
                            std::cout << "0";
                            gen_init_val_brackets_after(ivp->idx, ivp->shape);
                            ++ivp->idx;
                        }
                        // std::cout << "}";

                        ivp->level = old_level;
                    }
                }
            }
            else{
                if(type == 0){
                    exp_result_t exp_result;
                    exp->Dump2StringIR(&exp_result);

                    std::string result_pointer = gen_getelemptr_const_exp_koopa_code(
                        symbol_tables[stack_namespace.top()].get_array_pointer_int(ivp->ident),
                        ivp->idx++, ivp->shape
                    );

                    std::cout << "\tstore ";
                    if(exp_result.is_zero_depth){
                        std::cout << exp_result.result_number;
                    }
                    else{
                        std::cout << "%" << exp_result.result_id;
                    }
                    std::cout << ", " << result_pointer << std::endl;
                }
                else{
                    if(ivp->level == -1 && init_vals == nullptr){
                        int size_tot = 1;
                        for(int i = 0; i < ivp->shape.dim; ++i){
                            size_tot *= ivp->shape.array_size[i];
                        }

                        for(int i = 0; i < size_tot; ++i){
                            std::string result_pointer = gen_getelemptr_const_exp_koopa_code(
                                symbol_tables[stack_namespace.top()].get_array_pointer_int(ivp->ident),
                                i, ivp->shape
                            );
                            std::cout << "\tstore 0";
                            std::cout << ", " << result_pointer << std::endl;
                        }
                    }
                    else{
                        int old_level = ivp->level;
                        int cur_dim = ivp->shape.dim - 1;

                        assert(ivp->idx % ivp->shape.array_size[cur_dim] == 0);

                        int alignment = ivp->shape.array_size[cur_dim];
                        for(--cur_dim; cur_dim > old_level; --cur_dim){
                            if(ivp->idx % (alignment * ivp->shape.array_size[cur_dim]) != 0){
                                break;
                            }
                            alignment *= ivp->shape.array_size[cur_dim];
                        }
                        ivp->level = ++cur_dim;

                        if(init_vals != nullptr){
                            init_vals->Dump2StringIR(aux);
                        }
                        else{
                            std::string result_pointer = gen_getelemptr_const_exp_koopa_code(
                                symbol_tables[stack_namespace.top()].get_array_pointer_int(ivp->ident),
                                ivp->idx++, ivp->shape
                            );
                            std::cout << "\tstore 0";
                            std::cout << ", " << result_pointer << std::endl;
                        }

                        while(ivp->idx % alignment != 0){
                            std::string result_pointer = gen_getelemptr_const_exp_koopa_code(
                                symbol_tables[stack_namespace.top()].get_array_pointer_int(ivp->ident),
                                ivp->idx++, ivp->shape
                            );
                            std::cout << "\tstore 0";
                            std::cout << ", " << result_pointer << std::endl;
                        }

                        ivp->level = old_level;
                    }
                }
            }
        }
    }
};

class InitValsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_init_vals;

    void Dump() const override {
        std::cout << " InitValsAST { ";
        auto ii = vec_init_vals.begin();
        auto ie = vec_init_vals.end();
        for(; ii != ie; ++ii){
            (*ii)->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        init_val_param_t *ivp = (init_val_param_t *)aux;
        auto ii = vec_init_vals.begin();
        auto ie = vec_init_vals.end();
        if(ii != ie){
            (*ii)->Dump2StringIR(aux);
            ++ii;
        }
        for(; ii != ie; ++ii){
            if(ivp->is_global){
                std::cout << ", ";
            }
            (*ii)->Dump2StringIR(aux);
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
    int type;
    std::string b_type;
    std::string ident;
    std::unique_ptr<BaseAST> const_exps;

    void Dump() const override {
        std::cout << " FuncFParamAST { ";
        std::cout << " type: " << type << ", ";
        std::cout << " BType:" << b_type << ", ";
        std::cout << ident;
        if(type == 1){
            std::cout << " [ ";
            if(const_exps != nullptr){
                const_exps->Dump();
            }
            std::cout << " ] ";
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        func_f_param_result_t *param_r = (func_f_param_result_t *)aux;

        /* FURTHER: what if other types */
        assert(b_type == "int");
        std::string type_base = "i32";

        if(type == 1){
            if(const_exps != nullptr){
                const_exps->Dump2StringIR(&param_r->shape);
                std::cout << "@" << ident << ": " << "*";
                int dim = param_r->shape.dim;
                for(int i = dim - 1; i >= 0; --i){
                    std::cout << "[";
                }
                std::cout << type_base;
                for(int i = dim - 1; i >= 0; --i){
                    std::cout << ", " << param_r->shape.array_size[i] << "]";
                }
            }
            else{
                param_r->shape.dim = 0;
                std::cout << "@" << ident << ": " << "*" << type_base;
            }
        }
        else{
            std::cout << "@" << ident << ": " << type_base;
        }

        param_r->type = type;
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
                std::string type_base = "i32";

                if(params->params[i].type == 0){
                    symbol_tables[id].insert_var_func_param_int(ident, id);

                    std::cout << "\t" << symbol_tables[id].get_var_pointer_int(ident);
                    std::cout << " = alloc " << type_base << std::endl;

                    std::cout << "\tstore @" << ident << ", ";
                    std::cout << symbol_tables[id].get_var_pointer_int(ident);
                    std::cout << std::endl;
                }
                else{
                    int dim = params->params[i].shape.dim;

                    symbol_tables[id].insert_pointer_definition_int(ident, id, dim + 1);

                    if(dim == 0){
                        std::cout << "\t" << symbol_tables[id].get_pointer_pointer_int(ident);
                        std::cout << " = alloc " << "*" << type_base << std::endl;

                        std::cout << "\tstore @" << ident << ", ";
                        std::cout << symbol_tables[id].get_pointer_pointer_int(ident);
                        std::cout << std::endl;
                    }
                    else{
                        std::cout << "\t" << symbol_tables[id].get_pointer_pointer_int(ident);
                        std::cout << " = alloc " << "*";
                        for(int j = dim - 1; j >= 0; --j){
                            std::cout << "[";
                        }
                        std::cout << type_base;
                        for(int j = dim - 1; j >= 0; --j){
                            std::cout << ", ";
                            std::cout << params->params[i].shape.array_size[j];
                            std::cout << "]";
                        }
                        std::cout << std::endl;

                        std::cout << "\tstore @" << ident << ", ";
                        std::cout << symbol_tables[id].get_pointer_pointer_int(ident);
                        std::cout << std::endl;
                    }
                }
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

            assert(!l_val_result.is_const);
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
    std::unique_ptr<BaseAST> exps;

    void Dump() const override {
        std::cout << " LValAST { ";
        std::cout << ident;
        if(exps != nullptr){
            exps->Dump();
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        l_val_result_t *lval = (l_val_result_t *)aux;
        int cur_namespace = stack_namespace.top();
        if(exps == nullptr){
            if(lval->lhs){
                assert(symbol_tables[cur_namespace].bool_symbol_is_var_int(ident));
                lval->is_const = false;
                lval->pointer =
                    symbol_tables[cur_namespace].get_var_pointer_int(ident);
            }
            else{
                bool is_var = symbol_tables[cur_namespace].bool_symbol_is_var_int(ident);
                bool is_array = symbol_tables[cur_namespace].bool_symbol_is_array_int(ident);
                bool is_pointer = symbol_tables[cur_namespace].bool_symbol_is_pointer_int(ident);
                lval->is_const = !(is_var || is_array || is_pointer);
                if(is_var){
                    lval->pointer =
                        symbol_tables[cur_namespace].get_var_pointer_int(ident);

                    std::cout << "\t%" << result_id++ << " = load "
                            << lval->pointer
                            << std::endl;
                }
                else if(is_array){
                    lval->pointer =
                        symbol_tables[cur_namespace].get_array_pointer_int(ident);
                    std::cout << "\t%" << result_id++ << " = getelemptr "
                            << lval->pointer << ", 0"
                            << std::endl;
                }
                else if(is_pointer){
                    lval->pointer =
                        symbol_tables[cur_namespace].get_pointer_pointer_int(ident);
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
            /* array or pointer */
            bool is_array = symbol_tables[cur_namespace].bool_symbol_is_array_int(ident);
            bool is_pointer = symbol_tables[cur_namespace].bool_symbol_is_pointer_int(ident);
            assert(is_array || is_pointer);

            exps_result_t exps_result;
            exps->Dump2StringIR(&exps_result);

            lval->is_const = false;
            if(lval->lhs){
                /* Assign (lhs) */
                std::string pointer_lhs, pointer_rhs;
                int dim;
                if(is_array){
                    pointer_rhs = symbol_tables[cur_namespace].get_array_pointer_int(ident);
                    dim = symbol_tables[cur_namespace].get_array_dim_int(ident);
                }
                else if(is_pointer){
                    pointer_rhs = symbol_tables[cur_namespace].get_pointer_pointer_int(ident);
                    dim = symbol_tables[cur_namespace].get_pointer_dim_int(ident);
                }
                else{
                    assert(false);
                }
                assert(dim == exps_result.dim);
                for(int i = 0; i < exps_result.dim; ++i){
                    pointer_lhs = "%" + std::to_string(result_id++);
                    if(i == 0 && is_pointer){
                        std::cout << "\t" << pointer_lhs;
                        std::cout << " = load ";
                        std::cout << pointer_rhs << std::endl;
                        pointer_rhs = pointer_lhs;
                        pointer_lhs = "%" + std::to_string(result_id++);
                        std::cout << "\t" << pointer_lhs;
                        std::cout << " = getptr ";
                    }
                    else{
                        std::cout << "\t" << pointer_lhs;
                        std::cout << " = getelemptr ";
                    }
                    std::cout << pointer_rhs << ", ";
                    if(exps_result.idx[i].is_zero_depth){
                        std::cout << exps_result.idx[i].result_number;
                    }
                    else{
                        std::cout << "%" << exps_result.idx[i].result_id;
                    }
                    std::cout << std::endl;
                    pointer_rhs = pointer_lhs;
                }
                lval->pointer = pointer_lhs;
            }
            else{
                /* Get Value (rhs) */
                std::string pointer_lhs, pointer_rhs;
                int dim;
                if(is_array){
                    pointer_rhs = symbol_tables[cur_namespace].get_array_pointer_int(ident);
                    dim = symbol_tables[cur_namespace].get_array_dim_int(ident);
                }
                else if(is_pointer){
                    pointer_rhs = symbol_tables[cur_namespace].get_pointer_pointer_int(ident);
                    dim = symbol_tables[cur_namespace].get_pointer_dim_int(ident);
                }
                else{
                    assert(false);
                }
                for(int i = 0; i < exps_result.dim; ++i){
                    pointer_lhs = "%" + std::to_string(result_id++);
                    if(i == 0 && is_pointer){
                        std::cout << "\t" << pointer_lhs;
                        std::cout << " = load ";
                        std::cout << pointer_rhs << std::endl;
                        pointer_rhs = pointer_lhs;
                        pointer_lhs = "%" + std::to_string(result_id++);
                        std::cout << "\t" << pointer_lhs;
                        std::cout << " = getptr ";
                    }
                    else{
                        std::cout << "\t" << pointer_lhs;
                        std::cout << " = getelemptr ";
                    }
                    std::cout << pointer_rhs << ", ";
                    if(exps_result.idx[i].is_zero_depth){
                        std::cout << exps_result.idx[i].result_number;
                    }
                    else{
                        std::cout << "%" << exps_result.idx[i].result_id;
                    }
                    std::cout << std::endl;
                    pointer_rhs = pointer_lhs;
                }
                if(exps_result.dim < dim){
                    pointer_lhs = "%" + std::to_string(result_id++);
                    std::cout << "\t" << pointer_lhs;
                    std::cout << " = getelemptr ";
                    std::cout << pointer_rhs << ", " << 0;
                    std::cout << std::endl;
                    pointer_rhs = pointer_lhs;
                }
                else{
                    std::cout << "\t%" << result_id++ << " = load ";
                    std::cout << pointer_lhs << std::endl;
                }
                lval->pointer = pointer_lhs;
            }
        }
    }
};

class ExpsAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > vec_exps;

    void Dump() const override {
        std::cout << " ExpsAST { ";
        auto ii = vec_exps.begin();
        auto ie = vec_exps.end();
        for(; ii != ie; ++ii){
            std::cout << " [ ";
            (*ii)->Dump();
            std::cout << " ] ";
        }
        std::cout << " } ";
    }

    void Dump2StringIR(void *aux) const override {
        exps_result_t *exps_result = (exps_result_t *)aux;
        exps_result->dim = 0;
        exps_result->idx = std::vector<exp_result_t>();
        auto ii = vec_exps.begin();
        auto ie = vec_exps.end();
        for(; ii != ie; ++ii){
            exp_result_t exp_result;
            (*ii)->Dump2StringIR(&exp_result);
            exps_result->idx.push_back(exp_result);
            ++exps_result->dim;
        }
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

            if(!l_val_result.is_const){
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
