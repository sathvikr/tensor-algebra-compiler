//
//  error.cpp
//  Tensor Algebra Compiler
//
//

#include <stdio.h>
#include <iostream>
#include <string>

class Problem {
private:
    int index;
    std::string message;
public:
    Problem(int index, std::string message) {
        this->index = index;
        this->message = message;
        
        std::cout << message << std::endl;
    }
    
    int get_index() {
        return index;
    }
    
    std::string get_message() {
        return message;
    }
};

class Error : public Problem {
public:
    Error(int index, std::string message) : Problem(index, message) {
        exit(0);
    }
};

class LexicalError : public Error {
public:
    LexicalError(int index) : Error(index, {"Lexical error"}) {
        
    }
};

class SyntaxError : public Error {
public:
    SyntaxError(int index) : Error(index, {"Syntax error"}) {
        
    }
};

class SemanticError : public Error {
public:
    SemanticError(int index) : Error(index, {"Semantic error"}) {
        
    }
};

class IllegalIdentifierError : public SemanticError {
public:
    IllegalIdentifierError(int index) : SemanticError(index) {}
};

class LogicalError : public Error {
public:
    LogicalError(int index) : Error(index, {"Logical error"}) {
        
    }
};

class Warning : public Problem {};
