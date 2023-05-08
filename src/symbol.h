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

class SymbolTable {
private:
    std::map<std::string, symbol_table_entry_t> map_symbol2entry;

public:
    SymbolTable(){
        map_symbol2entry = std::map<std::string, symbol_table_entry_t>();
    }

    bool bool_symbol_exist(const std::string &s){
        return (map_symbol2entry.find(s) != map_symbol2entry.end());
    }

    bool bool_symbol_is_var(const std::string &s){
        assert(bool_symbol_exist(s));
        return map_symbol2entry[s].is_var;
    }

    void insert_const_definition_int(const std::string &s, int val_int){
        map_symbol2entry[s].is_var = false;
        map_symbol2entry[s].val_int = val_int;
    }

    int get_const_definition_int(const std::string &s){
        assert(!bool_symbol_is_var(s));
        return map_symbol2entry[s].val_int;
    }

    void insert_var_definition_int(const std::string &s){
        map_symbol2entry[s].is_var = true;
        map_symbol2entry[s].pointer = "@" + s;
    }

    std::string get_var_pointer_int(const std::string &s){
        assert(bool_symbol_is_var(s));
        return map_symbol2entry[s].pointer;
    }

    void clear_table(){
        map_symbol2entry.clear();
    }
};

#endif /**< src/symbol.h */
