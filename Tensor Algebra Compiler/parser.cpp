//
//  parser.cpp
//  Tensor Algebra Compiler
//
//

#include <stdio.h>
#include <iostream>
#include <regex>
#include <string>
#include <fstream>

#include "tokenizer.cpp"

class Parser {
private:
    std::regex const r_type = std::regex("int|float|tensor");
    std::regex const r_statements = std::regex("let");
    std::regex const r_binary_op = std::regex("[@+*-/^%]");
    std::regex const r_unary_op = std::regex("[~'-]");
    std::unordered_map<VarKind, std::string> const kind_to_string = { {VarKind::ARG, "arg"}, {VarKind::LOCAL, "local"}, {VarKind::GLOBAL, "global"}, {VarKind::NONE, "none"} };
    std::unordered_map<TokenType, DataType> const ttype_to_dtype = { {TokenType::T_INT, DataType::INT}, {TokenType::T_FLOAT, DataType::FLOAT} };
    std::unordered_map<char, int> const precedence_map = { {'^', 3}, {'/', 2}, {'*', 2}, {'+', 1}, {'-', 1} };
    std::unordered_map<char, bool> const left_associativity_map = { {'^', false}, {'/', true}, {'*', true}, {'+', true}, {'-', true} };
    
    std::ifstream in;
    std::ofstream out;
    Tokenizer tokenizer;
    
    int indents, num_labels;
    SymbolTable symbol_table;
public:
    Parser(std::string& ifname, std::string ofname) : tokenizer(ifname) {
        in = std::ifstream(ifname);
        out = std::ofstream(ofname);
        indents = 0;
        num_labels = 0;
    }
    
    void write_line(std::string line) {
        for (int i = 0; i < indents; i++) {
            out << "\t";
        }
        
        out << line << std::endl;
    }
    
    void write_identifier(std::string identifier, std::string kind) {
        std::string running_index = std::to_string(symbol_table.get_running_index());
        write_line("<" + kind + "_identifier" + running_index + "> " + identifier + " </" + kind + "_identifier" + running_index + ">");
    }
    
    std::string get_current_token_repr() {
        switch (tokenizer.token_type()) {
            case TokenType::T_KEYWORD:
                return "<keyword> " + tokenizer.get_current_token() + " </keyword>";
            case TokenType::T_SYMBOL:
                if (regex_match(std::string(1, tokenizer.symbol()), std::regex("[<>\"&]"))) {
                    return "<symbol> " + tokenizer.get_altered_symbols().at({tokenizer.symbol()}) + "; </symbol>";
                } else {
                    return "<symbol> " + std::string(1, tokenizer.symbol()) + " </symbol>";
                }
            case TokenType::T_IDENTIFIER:
                return "<identifier>" + tokenizer.identifier() + " </identifier>";
            case TokenType::T_INT:
                return "<int_const> " + tokenizer.get_current_token() + " </int_const>";
            case TokenType::T_FLOAT:
                return "<float_const> " + tokenizer.get_current_token() + " </float_const>";
            default:
                return "";
        }
    }
    
    void advance() {
        write_line(get_current_token_repr());
        tokenizer.advance();
    }
    
    void advance_identifier(std::string kind) {
        write_identifier(tokenizer.identifier(), kind);
        tokenizer.advance();
    }
    
    std::string eat(std::regex str) {
        std::string eaten = tokenizer.get_current_token();
        
        if (regex_match(tokenizer.get_current_token(), str)) {
            advance();
        } else {
            SyntaxError(-1);
        }
        
        return eaten;
    }
    
    std::string eat_next_identifier(std::string kind) {
        std::string eaten = tokenizer.get_current_token();
        
        if (tokenizer.token_type() == TokenType::T_IDENTIFIER) {
            advance_identifier(kind);
        } else {
            SyntaxError(-1);
        }
        
        return eaten;
    }
    
    std::string eat_if_next(std::regex str) {
        std::string eaten = tokenizer.get_current_token();
        
        if (regex_match(tokenizer.get_current_token(), str)) {
            advance();
        }
        
        return eaten;
    }
    
    bool is_right_associative(char op) {
        return op == '^';
    }
    
    ProgramNode parse_compilation_unit() {
        write_line("<compilation_unit>");
        indents++;
        ProgramNode compilation_unit = ProgramNode("compilation_unit");
        ProgramNode statements = parse_statements();
        std::shared_ptr<ASTNode> statements_ptr = std::make_shared<ProgramNode>(statements);
        compilation_unit.add_child(statements_ptr);
//
        indents--;
        write_line("</compilation_unit>");
//
        return compilation_unit;
    }
    
    ProgramNode parse_statements() {
        write_line("<statements>");
        indents++;
        ProgramNode statements = ProgramNode("statements");

        symbol_table = SymbolTable();

        while (regex_match(tokenizer.get_current_token(), r_statements)) {
            if (tokenizer.get_current_token() == "let") {
                VarDecNode var_dec = parse_var_dec();
                std::shared_ptr<ASTNode> var_dec_ptr = std::make_shared<VarDecNode>(var_dec);
                statements.add_child(var_dec_ptr);
            } else {
                SyntaxError(-1);
            }
        }

        indents--;
        write_line("</statements>");

        return statements;
    }
//
    VarDecNode parse_var_dec() {
        write_line("<var_dec>");
        indents++;

        eat(std::regex("let"));
        std::string var_type = eat(r_type);

//        while (tokenizer.get_current_token() == "[") {
//            eat(std::regex("\\["));
//            parse_expression());
//            eat(std::regex("\\]"));
//        }

        std::string var_name = eat_next_identifier(kind_to_string.at(VarKind::LOCAL));
        symbol_table.define(var_name, var_type, VarKind::LOCAL);

        eat(std::regex("="));
        std::shared_ptr<ExpressionNode> rhs = parse_expression();
        eat(std::regex(";"));

        indents--;
        write_line("</var_dec>");
        return VarDecNode(var_name, var_type, VarKind::LOCAL, rhs);
    }
//
    std::shared_ptr<ExpressionNode> parse_expression() {
        write_line("<expression>");
        indents++;
        std::shared_ptr<ExpressionNode> expression = parse_term();

        while (regex_match(tokenizer.get_current_token(), std::regex("[+-]"))) {
            Token op = Token(tokenizer.get_current_token(), tokenizer.token_type());
            advance();
            std::shared_ptr<ExpressionNode> term = parse_term();
            ExpressionNode n = ExpressionNode(op, expression, term);
            expression = std::make_shared<ExpressionNode>(n);
        }

        indents--;
        write_line("</expression>");

        return expression;
    }
//
    std::shared_ptr<ExpressionNode> parse_term() {
        write_line("<term>");
        indents++;
        std::shared_ptr<ExpressionNode> term = parse_factor();

        while (regex_match(tokenizer.get_current_token(), std::regex("[*/]"))) {
            Token op = Token(tokenizer.get_current_token(), tokenizer.token_type());
            advance();
            std::shared_ptr<ExpressionNode> term2 = parse_term();
            ExpressionNode n = ExpressionNode(op, term, term2);
            term = std::make_shared<ExpressionNode>(n);
        }

        indents--;
        write_line("</term>");

        return term;
    }
//
    std::shared_ptr<ExpressionNode> parse_factor() {
        write_line("<factor>");
        indents++;
        std::shared_ptr<ExpressionNode> factor;

        if (tokenizer.get_current_token() == "(") {
            advance();
            factor = parse_expression();
            eat(std::regex("\\)"));
        } else {
            factor = parse_primary();
        }

        indents--;
        write_line("</factor>");

        return factor;
    }
//
    std::shared_ptr<ExpressionNode> parse_primary() {
        write_line("<primary>");
        indents++;
        ExpressionNode n = ExpressionNode();
        std::shared_ptr<ExpressionNode> primary = std::make_shared<ExpressionNode>(n);

        if (tokenizer.token_type() == TokenType::T_INT ||
            tokenizer.token_type() == TokenType::T_FLOAT) {
            ScalarNode scalar_node = ScalarNode(tokenizer.get_current_token(), ttype_to_dtype.at(tokenizer.token_type()));
            primary = std::make_shared<ScalarNode>(scalar_node);
            advance();
        } else if (tokenizer.token_type() == TokenType::T_IDENTIFIER) {
            VarKind var_kind = symbol_table.kind_of(tokenizer.get_current_token());
            IndentifierNode identifier_node = IndentifierNode(tokenizer.get_current_token());
            primary = std::make_shared<IndentifierNode>(identifier_node);
            eat_next_identifier(kind_to_string.at(var_kind));
        } else if (tokenizer.get_current_token() == "{") {
            primary = parse_tensor();
        } else if (regex_match(tokenizer.get_current_token(), r_unary_op)) {
            Token op = Token(tokenizer.get_current_token(), tokenizer.token_type());
            advance();
            std::shared_ptr<ExpressionNode> term = parse_term();
            primary->set_value(op);
            primary->set_right(*term);
        }

        indents--;
        write_line("</primary>");

        return primary;
    }
    
    std::shared_ptr<TensorNode> parse_tensor(std::shared_ptr<TensorNode> curr_node=std::make_shared<TensorNode>(TensorNode()), int level=0, int prev_level=0) {
        while (tokenizer.get_current_token() == "{") {
            prev_level = level;
            level++;
            
            std::cout << prev_level << ", " << level << "|" << tokenizer.get_current_token() << std::endl;
            
            if (level < prev_level) {
                curr_node->set_first_child(parse_tensor(curr_node=curr_node, level=level, prev_level=prev_level));
            } else if (level == prev_level) {
                curr_node->set_next_sibling(parse_tensor(curr_node=curr_node, level=level, prev_level=prev_level));
            }
            
            eat(std::regex("\\{"));
        }
        
        while (tokenizer.get_current_token() != "}") {
            if (tokenizer.token_type() == TokenType::T_INT ||
                tokenizer.token_type() == TokenType::T_FLOAT) {
                if (tokenizer.get_current_token() != "0") {
                    ScalarNode leaf_node = ScalarNode(tokenizer.get_current_token(), ttype_to_dtype.at(tokenizer.token_type()));
                    curr_node->set_first_child(std::make_shared<TensorNode>(leaf_node));
                }
            }
            
            advance();
            eat_if_next(std::regex(","));
        }
        
        while (tokenizer.get_current_token() == "}") {
            level--;
            
            eat(std::regex("\\}"));
            eat_if_next(std::regex(","));
        }
        
        return curr_node;
    }
//
//    std::shared_ptr<TensorNode> parse_tensor(bool is_first_pass=true) {
//        if (!is_first_pass) {
//            write_line("<tensor>");
//            indents++;
//        }
//
//        int parent_index = 0;
//
//        /*
//
//         {3.0, 2.0}
//
//         0 -> 1
//         |    |
//         3.0  2.0
//
//         */
//
//        while (tokenizer.get_current_token() == "{") {
////            std::cout << tokenizer.get_current_token() << std::endl;
//            eat(std::regex("\\{"));
////            curr_tensor->add_index(parent_index);
////            curr_tensor->set_index(parent_index);
////            std::shared_ptr<TensorNode> n = parse_tensor(curr_tensor=curr_tensor, is_first_pass=false);
//            parse_tensor(is_first_pass=false);
////
////            if (parent_index == 0) {
////                curr_tensor->set_first_child(n);
////            } else {
////                curr_tensor->set_next_sibling(n);
////            }
////            parse_tensor(curr_tensor->get_subtensor(parent_index), false);
//            eat(std::regex("\\}"));
//            eat_if_next(std::regex(","));
//            parent_index++;
//        }
//
//        int leaf_index = 0;
//
//        if (!is_first_pass) {
//            while (tokenizer.get_current_token() != "}") {
////                if (tokenizer.token_type() == TokenType::T_INT ||
////                    tokenizer.token_type() == TokenType::T_FLOAT) {
////                    if (tokenizer.get_current_token() != "0") {
////                        std::shared_ptr<TensorNode> n = std::make_shared<ScalarNode>(ScalarNode(tokenizer.get_current_token(), ttype_to_dtype.at(tokenizer.token_type())));
//////                        curr_tensor->add_value(leaf_index, n);
////                        if (leaf_index == 0) {
////                            curr_tensor->set_first_child(n);
////                        } else {
////                            curr_tensor->set_next_sibling(n);
////                        }
////                    }
////
////                    leaf_index++;
////                }
//
//                advance();
//            }
//
//            indents--;
//            write_line("</tensor>");
//        }
//
//        return nullptr;
//    }
//
};
