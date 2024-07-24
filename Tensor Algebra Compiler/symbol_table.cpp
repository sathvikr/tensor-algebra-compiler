//
//  symbol_table.cpp
//  Tensor Algebra Compiler
//
//

#include <stdio.h>
#include <unordered_map>

#include "token.cpp"
#include "var_info.cpp"

class SymbolTable {
private:
    int running_index;
    std::unordered_map<std::string, VarInfo> table;
public:
    SymbolTable() {
        table = std::unordered_map<std::string, VarInfo>();
        running_index = 0;
    }

    int get_running_index() {
        return running_index;
    }
    
    void define(std::string name, std::string type, VarKind kind) {
        table.insert({name, VarInfo(type, kind, running_index)});
        running_index++;
    }
    
    int var_count(VarKind kind) {
        int count = 0;
        
        for (auto var_info : table) {
            if (var_info.second.get_kind() == kind) {
                count++;
            }
        }
        
        return count;
    }
    
    VarKind kind_of(std::string name) {
        if (table.find(name) != table.end()) {
            return table.at(name).get_kind();
        }
        
        return VarKind::NONE;
    }
    
    std::string type_of(std::string name) {
        if (table.find(name) != table.end()) {
            return table.at(name).get_type();
        }
        
        return "";
    }
    
    int index_of(std::string name) {
        if (table.find(name) != table.end()) {
            return table.at(name).get_index();
        }
        
        return -1;
    }
};
