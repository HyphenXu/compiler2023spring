#ifndef SYMBOL_H
#define SYMBOL_H

#include <map>
#include <string>
#include <cassert>

typedef struct{
    bool is_var;
    int val_int;
    std::string pointer;
} symbol_table_entry_t;

class SymbolTable;

static std::map<int, SymbolTable> map_blockID2symbolTable;
static std::stack<int> stack_namespace;

class SymbolTable {
private:
    std::map<std::string, symbol_table_entry_t> map_symbol2entry;
    int parent_block_id;

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
                    (parent_block_id == -1)
                    ? false
                    : map_blockID2symbolTable[parent_block_id].bool_symbol_exist(s)
                )
                ;
    }

    symbol_table_entry_t &get_st_entry(const std::string &s){
        // assert(bool_symbol_exist(s));
        if(map_symbol2entry.find(s) != map_symbol2entry.end()){
            return map_symbol2entry[s];
        }
        else{
            assert(parent_block_id != -1);
            return map_blockID2symbolTable[parent_block_id].get_st_entry(s);
        }
    }

    bool bool_symbol_is_var(const std::string &s){
        assert(bool_symbol_exist(s));
        return get_st_entry(s).is_var;
    }

    void insert_const_definition_int(const std::string &s, int val_int){
        map_symbol2entry[s].is_var = false;
        map_symbol2entry[s].val_int = val_int;
    }

    int get_const_definition_int(const std::string &s){
        assert(!bool_symbol_is_var(s));
        return get_st_entry(s).val_int;
    }

    void insert_var_definition_int(const std::string &s, int cur_block){
        map_symbol2entry[s].is_var = true;
        map_symbol2entry[s].pointer = "@" + s + "_" + std::to_string(cur_block);
    }

    std::string get_var_pointer_int(const std::string &s){
        assert(bool_symbol_is_var(s));
        return get_st_entry(s).pointer;
    }

    void clear_table(){
        map_symbol2entry.clear();
    }
};

#endif /**< src/symbol.h */
