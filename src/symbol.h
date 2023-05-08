#ifndef SYMBOL_H
#define SYMBOL_H

#include <map>
#include <string>
#include <cassert>

class SymbolTable {
private:
    std::map<std::string, int> map_string2int;
    std::map<std::string, std::string> map_string2type;

public:
    SymbolTable(){
        map_string2int = std::map<std::string, int>();
        map_string2type = std::map<std::string, std::string>();
    }

    void insert_definition(const std::string &s, int val){
        map_string2int[s] = val;
    }

    bool bool_exist(const std::string &s){
        return (map_string2int.find(s) != map_string2int.end());
    }

    int get_definition(const std::string &s){
        assert(bool_exist(s));
        return map_string2int[s];
    }

    void clear_table(){
        map_string2int.clear();
        map_string2type.clear();
    }
};

#endif /**< src/symbol.h */
