#ifndef SYMBOL_H
#define SYMBOL_H

#include <map>
#include <string>
#include <cassert>

typedef enum {
    SYMBOL_TYPE_NST_INT,
    SYMBOL_TYPE_VAR_INT,
    SYMBOL_TYPE_FUNCTION,
} symbol_type_t;

typedef struct{
    symbol_type_t type;
    bool is_var;
    int val_int;
    std::string pointer;

    std::string type_return;
} symbol_table_entry_t;

class SymbolTable;

const int ROOT_NAMESPACE_ID = -1;
const int GLOBAL_NAMESPACE_ID = 0;
static std::map<int, SymbolTable> map_blockID2symbolTable;
static std::stack<int> stack_namespace;

class SymbolTable {
private:
    std::map<std::string, symbol_table_entry_t> map_symbol2entry;
    int parent_block_id;

    symbol_table_entry_t &get_st_entry(const std::string &s){
        // assert(bool_symbol_exist(s));
        if(map_symbol2entry.find(s) != map_symbol2entry.end()){
            return map_symbol2entry[s];
        }
        else{
            assert(parent_block_id != ROOT_NAMESPACE_ID);
            return map_blockID2symbolTable[parent_block_id].get_st_entry(s);
        }
    }

public:
    SymbolTable(){
        map_symbol2entry = std::map<std::string, symbol_table_entry_t>();
    }

    void update_parent_block(int pb_id){
        parent_block_id = pb_id;
    }

    bool bool_symbol_exist_local(const std::string &s){
        return (map_symbol2entry.find(s) != map_symbol2entry.end());
    }

    bool bool_symbol_exist(const std::string &s){
        return (map_symbol2entry.find(s) != map_symbol2entry.end())
                ? true
                : (
                    (parent_block_id == ROOT_NAMESPACE_ID)
                    ? false
                    : map_blockID2symbolTable[parent_block_id].bool_symbol_exist(s)
                )
                ;
    }

    bool bool_symbol_is_var(const std::string &s){
        assert(bool_symbol_exist(s));
        return get_st_entry(s).is_var;
    }

    bool bool_symbol_is_func(const std::string &s){
        assert(bool_symbol_exist(s));
        return get_st_entry(s).type == SYMBOL_TYPE_FUNCTION;
    }

    void insert_const_definition_int(const std::string &s, int val_int){
        map_symbol2entry[s].is_var = false;
        map_symbol2entry[s].val_int = val_int;
    }

    int get_const_definition_int(const std::string &s){
        /* TODO: change anything related to is_var */
        assert(!bool_symbol_is_var(s));
        return get_st_entry(s).val_int;
    }

    void insert_var_definition_int(const std::string &s, int cur_block){
        map_symbol2entry[s].is_var = true;
        map_symbol2entry[s].pointer = "@" + s + "_" + std::to_string(cur_block);
    }

    void insert_var_func_param_int(const std::string &s, int cur_block){
        map_symbol2entry[s].is_var = true;
        map_symbol2entry[s].pointer = "%" + s + "_" + std::to_string(cur_block);
    }

    std::string get_var_pointer_int(const std::string &s){
        assert(bool_symbol_is_var(s));
        return get_st_entry(s).pointer;
    }

    void insert_func_def(const std::string &s, const std::string &t){
        map_symbol2entry[s].type = SYMBOL_TYPE_FUNCTION;
        map_symbol2entry[s].type_return = t;
    }

    std::string get_func_return_type(const std::string &s){
        assert(bool_symbol_is_func(s));
        return get_st_entry(s).type_return;
    }

    void clear_table(){
        map_symbol2entry.clear();
    }

};

#endif /**< src/symbol.h */