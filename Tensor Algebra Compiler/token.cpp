//
//  token.cpp
//  Tensor Algebra Compiler
//
//

#include <stdio.h>
#include <string>

#include "token_type.cpp"

class Token {
private:
    std::string token;
    TokenType token_type;

public:
    Token() {
        token = "";
        token_type = TokenType::T_NONE;
    }
    
    Token(std::string token) {
        this->token = token;
        token_type = TokenType::T_NONE;
    }
    
    Token(std::string token, TokenType token_type) {
        this->token = token;
        this->token_type = token_type;
    }
    
    std::string get_token() {
        return token;
    }
    
    void set_token(std::string token) {
        this->token = token;
    }
    
    TokenType get_token_type() {
        return token_type;
    }
    
    void set_token_type(TokenType token_type) {
        this->token_type = token_type;
    }
};
