//
//  token_type.cpp
//  Tensor Algebra Compiler
//
//

#include <stdio.h>

enum class TokenType {
    T_KEYWORD,
    T_SYMBOL,
    T_IDENTIFIER,
    T_INT,
    T_FLOAT,
    T_NONE
};

