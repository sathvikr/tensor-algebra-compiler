//
//  var_info.cpp
//  Tensor Algebra Compiler
//
//

#include <stdio.h>
#include <string>

#include "var_kind.cpp"

class VarInfo {
private:
    std::string type;
    VarKind kind;
    int index;
public:
    VarInfo(std::string type, VarKind kind, int index) {
        this->type = type;
        this->kind = kind;
        this->index = index;
    }
    
    std::string get_type() {
        return type;
    }
    
    VarKind get_kind() {
        return kind;
    }
    
    int get_index() {
        return index;
    }
    
};
