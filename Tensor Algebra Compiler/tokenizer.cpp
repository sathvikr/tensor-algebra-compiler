//
//  tokenizer.cpp
//  Tensor Algebra Compiler
//
//

#include <regex>
#include <fstream>
#include <unordered_map>
#include <iostream>
#include "keyword.cpp"
#include "node.cpp"

class Tokenizer {
private:
    std::regex const keywords = std::regex("let|int|float|tensor");
    std::regex const symbols = std::regex("[!@'{}().,;+*-/^%=~]|[[\\]]");
    std::unordered_map<TokenType, std::string> const type_to_string = { {TokenType::T_KEYWORD, "t_keyword"}, {TokenType::T_SYMBOL, "t_symbol"}, {TokenType::T_IDENTIFIER, "t_identifier"}, {TokenType::T_INT, "t_int"}, {TokenType::T_FLOAT, "t_float"}, {TokenType::T_NONE, "t_none"} };
    std::unordered_map<std::string, Keyword> const string_to_keyword = { {"let", Keyword::LET}, {"int", Keyword::INT}, {"float", Keyword::FLOAT}, {"tensor", Keyword::TENSOR} };
    std::unordered_map<std::string, std::string> const altered_symbols = { {"<", "&lt"}, {">", "&gt"}, {"\"", "&quot"}, {"&", "&amp"} };
    
    std::string content;
    int current_index;
    std::string current_char;
    std::string current_token;
public:
    Tokenizer(std::string ifname) {
        std::ifstream in(ifname);
        content = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        current_index = -1;
        current_char = "";
        current_token = "";
        advance();        
    }
    
    bool has_more_tokens() {
        return current_index == -1 || current_index <= content.size();
    };
    
    void next_char() {
        current_index++;
        
        if (current_index < content.size()) {
            current_char = {content[current_index]};
        }
    };
    
    void advance() {
        current_token = "";
        
        while (has_more_tokens()) {            
            if (current_char == "" || isspace(current_char[0])) {
                if (current_token != "") { break; }
                
                next_char();
            } else if (current_char == "#") {
                do {
                    next_char();
                } while (current_char != "\n" && current_char != "\r");
            } else if (regex_match(current_char, std::regex("\\.|[0-9]"))) {
                if (!regex_match(current_token, std::regex("\\d*\\.?\\d*"))) {
                    if (current_token != "" && regex_match(std::string(1, current_token[0]), std::regex("[0-9]"))) {
                        if (regex_match(current_char, std::regex("[a-zA-Z_]"))) { break; }
                    }
                                    
                    current_token += current_char;
                    next_char();
                } else {
                    if (current_char == "." && current_token.find(".") != std::string::npos) { break; }
                    
                    current_token += current_char;
                    next_char();
                }
            } else if (regex_match(current_char, symbols)) {
                if (current_token != "") { break; }
                
                current_token += current_char;
                next_char();
                break;
            } else if (regex_match(current_char, std::regex("[a-zA-Z0-9_]"))) {
                if (current_token == ".") { break; }
                if (current_token != "" && regex_match(std::string(1, current_token[0]), std::regex("[0-9]"))) {
                    if (regex_match(current_char, std::regex("[a-zA-Z_]"))) {
                        LexicalError(-1);
                    }
                }
                                
                current_token += current_char;
                next_char();
            } else {
                break;
            }
        }
    };
    
    TokenType token_type() {
        if (regex_match(current_token, symbols)) {
            return TokenType::T_SYMBOL;
        } else if (regex_match(current_token, std::regex("[a-zA-Z_][a-zA-Z0-9_]*"))) {
//            cout << current_token << endl;
            
            if (regex_match(current_token, keywords)) {
                return TokenType::T_KEYWORD;
            } else {
                return TokenType::T_IDENTIFIER;
            }
        } else if (regex_match(current_token, std::regex("\\d*\\.\\d+"))) {
            return TokenType::T_FLOAT;
        } else if (regex_match(current_token, std::regex("[0-9]+"))) {
            return TokenType::T_INT;
        }
        
        return TokenType::T_NONE;
    };
    
    Keyword keyword() {
        return string_to_keyword.at(current_token);
    };

    char symbol() {
        return current_token[0];
    };

    std::string identifier() {
        return current_token;
    };

    int int_val() {
        return stoi(current_token);
    };
    
    float float_val() {
        return stof(current_token);
    }
    
    Token get_current_token_obj() {
        return Token(current_token, token_type());
    }
    
    std::string get_current_token() {
        return current_token;
    }
    
    std::string get_current_token_repr() {
        std::string t_type = type_to_string.at(token_type());
        
        return "<" + t_type + "> " + current_token + " </" + t_type + ">";
    };
    
    std::unordered_map<std::string, std::string> get_altered_symbols() {
        return altered_symbols;
    }
};
