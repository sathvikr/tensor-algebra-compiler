//
//  vm_writer.cpp
//  Tensor Algebra Compiler
//
//

#include <stdio.h>
#include <fstream>

class VMWriter {
public:
    static void write_newline(std::ofstream& out) {
        out << std::endl;
    }
    
    static void write_push(std::ofstream& out, std::string segment, double n) {
        out << "push " << segment << " " << n << std::endl;
    }
    
    static void write_pop(std::ofstream& out, std::string segment, int n) {
        out << "pop " << segment << " " << n << std::endl;
    }
    
    static void write_malloc(std::ofstream& out, std::size_t size) {
        write_push(out, "constant", size);
        write_call(out, "Memory.alloc", 1);
    }
    
    static void write_call(std::ofstream& out, std::string func_name, int n_args) {
        out << "call " << func_name << " " << n_args << std::endl;
    }
};
