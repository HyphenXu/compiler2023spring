#ifndef SYMBOL_H
#define SYMBOL_H

#include <map>
#include <string>
#include <cassert>
#include <stack>
#include <iostream>

typedef enum{
    SYMBOL_TYPE_CONST_INT,
    SYMBOL_TYPE_VAR_INT,
    SYMBOL_TYPE_FUNCTION,
    SYMBOL_TYPE_ARRAY_INT,
} symbol_type_t;

typedef struct{
    symbol_type_t type;

    int val_const_int;

    std::string var_pointer_int;

    std::string func_type_return;

    std::string array_pointer_int;
} symbol_table_entry_t;

class SymbolTable;
typedef SymbolTable symbol_table_t;
typedef std::map<int, symbol_table_t> symbol_tables_t;

const int ROOT_NAMESPACE_ID = -1;
const int GLOBAL_NAMESPACE_ID = 0;

static symbol_tables_t symbol_tables;
static std::stack<int> stack_namespace;

class SymbolTable {
private:
    std::map<std::string, symbol_table_entry_t> map_symbol2entry;
    int parent_namespace;

    symbol_table_entry_t &get_st_entry(const std::string &s){
        // assert(bool_symbol_exist(s));
        if(map_symbol2entry.find(s) != map_symbol2entry.end()){
            return map_symbol2entry[s];
        }
        else{
            assert(parent_namespace != ROOT_NAMESPACE_ID);
            return symbol_tables[parent_namespace].get_st_entry(s);
        }
    }

public:
    SymbolTable() = default;

    SymbolTable(int p_ns){
        map_symbol2entry = std::map<std::string, symbol_table_entry_t>();
        parent_namespace = p_ns;
    }

    bool bool_symbol_exist_local(const std::string &s){
        return (map_symbol2entry.find(s) != map_symbol2entry.end());
    }

    bool bool_symbol_exist(const std::string &s){
        return (map_symbol2entry.find(s) != map_symbol2entry.end())
                ? true
                : (
                    (parent_namespace == ROOT_NAMESPACE_ID)
                    ? false
                    : symbol_tables[parent_namespace].bool_symbol_exist(s)
                )
                ;
    }

    bool bool_symbol_is_const_int(const std::string &s){
        assert(bool_symbol_exist(s));
        return get_st_entry(s).type == SYMBOL_TYPE_CONST_INT;
    }

    bool bool_symbol_is_var_int(const std::string &s){
        assert(bool_symbol_exist(s));
        return get_st_entry(s).type == SYMBOL_TYPE_VAR_INT;
    }

    bool bool_symbol_is_func(const std::string &s){
        assert(bool_symbol_exist(s));
        return get_st_entry(s).type == SYMBOL_TYPE_FUNCTION;
    }

    bool bool_symbol_is_array_int(const std::string &s){
        assert(bool_symbol_exist(s));
        return get_st_entry(s).type == SYMBOL_TYPE_ARRAY_INT;
    }

    void insert_const_definition_int(const std::string &s, int val_const_int){
        map_symbol2entry[s].type = SYMBOL_TYPE_CONST_INT;
        map_symbol2entry[s].val_const_int = val_const_int;
    }

    int get_const_definition_int(const std::string &s){
        std::cerr << "?" <<  s << "?" << "\n";
        assert(bool_symbol_is_const_int(s));
        return get_st_entry(s).val_const_int;
    }

    void insert_var_definition_int(const std::string &s, int cur_ns){
        map_symbol2entry[s].type = SYMBOL_TYPE_VAR_INT;
        map_symbol2entry[s].var_pointer_int = "@" + s + "_" + std::to_string(cur_ns);
    }

    void insert_var_func_param_int(const std::string &s, int cur_ns){
        map_symbol2entry[s].type = SYMBOL_TYPE_VAR_INT;
        map_symbol2entry[s].var_pointer_int = "%" + s + "_" + std::to_string(cur_ns);
    }

    std::string get_var_pointer_int(const std::string &s){
        assert(bool_symbol_is_var_int(s));
        return get_st_entry(s).var_pointer_int;
    }

    void insert_func_def(const std::string &s, const std::string &t){
        map_symbol2entry[s].type = SYMBOL_TYPE_FUNCTION;
        map_symbol2entry[s].func_type_return = t;
    }

    std::string get_func_return_type(const std::string &s){
        assert(bool_symbol_is_func(s));
        return get_st_entry(s).func_type_return;
    }

    void insert_array_definition_int(const std::string &s, int cur_ns){
        map_symbol2entry[s].type = SYMBOL_TYPE_ARRAY_INT;
        map_symbol2entry[s].array_pointer_int = "@" + s + "_" + std::to_string(cur_ns);
    }

    std::string get_array_pointer_int(const std::string &s){
        assert(bool_symbol_is_array_int(s));
        return get_st_entry(s).array_pointer_int;
    }

    void clear_table(){
        map_symbol2entry.clear();
    }

    static void insert_lib_func_def(){
        static bool is_lib_func_defined = false;
        if(is_lib_func_defined){
            return;
        }
        is_lib_func_defined = true;

        SymbolTable &st = symbol_tables[GLOBAL_NAMESPACE_ID];

        /* decl @getint(): i32 */
        st.insert_func_def("getint", "int");
        std::cout << "decl @" << "getint" << "(";
        std::cout << ")" << ": " << "i32" << std::endl;

        /* decl @getch(): i32 */
        st.insert_func_def("getch", "int");
        std::cout << "decl @" << "getch" << "(";
        std::cout << ")" << ": " << "i32" << std::endl;

        /* decl @getarray(*i32): i32 */
        st.insert_func_def("getarray", "int");
        std::cout << "decl @" << "getarray" << "(";
        std::cout << "*i32";
        std::cout << ")" << ": " << "i32" << std::endl;

        /* decl @putint(i32) */
        st.insert_func_def("putint", "void");
        std::cout << "decl @" << "putint" << "(";
        std::cout << "i32";
        std::cout << ")" << std::endl;

        /* decl @putch(i32) */
        st.insert_func_def("putch", "void");
        std::cout << "decl @" << "putch" << "(";
        std::cout << "i32";
        std::cout << ")" << std::endl;

        /* decl @putarray(i32, *i32) */
        st.insert_func_def("putarray", "void");
        std::cout << "decl @" << "putarray" << "(";
        std::cout << "i32" << ", " << "*i32";
        std::cout << ")" << std::endl;

        /* decl @starttime() */
        st.insert_func_def("starttime", "void");
        std::cout << "decl @" << "starttime" << "(";
        std::cout << ")" << std::endl;

        /* decl @stoptime() */
        st.insert_func_def("stoptime", "void");
        std::cout << "decl @" << "stoptime" << "(";
        std::cout << ")" << std::endl;

        std::cout << std::endl;
    }
};

#endif /**< src/symbol.h */
