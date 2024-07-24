//
//  code_generator.cpp
//  Tensor Algebra Compiler
//
//

#include <stdio.h>
#include "parser.cpp"
#include "tables.cpp"


class CodeGenerator {
private:
    ProgramNode ast;
    SymbolTable symbol_table;
    std::ofstream out;
public:
    CodeGenerator(ProgramNode n, std::string ofname) : ast(n) {
        out = std::ofstream(ofname);
    }
    
    void generate_code() {
        codegen_helper(ast);
    }
    
    void codegen_helper(ASTNode& n) {
        std::cout << n.get_token().get_token() << std::endl;
        
        n.codegen(out, symbol_table);
        
        for (std::shared_ptr<ASTNode> child : n.get_children()) {
            codegen_helper(*child);
        }
    }
};
