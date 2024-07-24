//
//  node.cpp
//  Tensor Algebra Compiler
//
//

#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include "data_type.cpp"
#include "symbol_table.cpp"
#include "virtual_segment.cpp"
#include "error.cpp"
#include "vm_writer.cpp"

std::unordered_map<DataType, TokenType> const dtype_to_ttype = { {DataType::INT, TokenType::T_INT}, {DataType::FLOAT, TokenType::T_FLOAT} };
std::unordered_map<VarKind, std::string> const vkind_to_vsegment = { {VarKind::ARG, "argument"}, {VarKind::LOCAL, "local"}, {VarKind::GLOBAL, "global"} };

class ASTNode {
private:
    Token token;
    std::vector<std::shared_ptr<ASTNode>> children;
public:
    ASTNode() {
        token = Token();
    }
    
    ASTNode(Token token) {
        this->token = token;
    }
    
//    ASTNode(ASTNode& other) {
//        token = other.get_token();
//        children = other.get_children();
//    }
    
    virtual ~ASTNode() = default;

    Token get_token() {
        return token;
    }

    void set_token(Token token) {
        this->token = token;
    }

    std::vector<std::shared_ptr<ASTNode>> const& get_children() {
        return children;
    }

    void add_child(std::shared_ptr<ASTNode>& child) {
        children.push_back(std::move(child));
    }

    void write_line(std::string line, int indents) {
        for (int i = 0; i < indents; i++) {
            std::cout << "\t";
        }

        std::cout << line << std::endl;
    }

    virtual void print(int indents=0) = 0;
    
    virtual void codegen(std::ofstream& out, SymbolTable& symbol_table) = 0;
};

class ProgramNode : public ASTNode {
private:
    std::string type;
public:
    ProgramNode(std::string t) : ASTNode(Token(t)) {
        type = t;
    }

    std::string get_type() {
        return type;
    }
    
    void print(int indents=0) override {
        write_line("<" + get_token().get_token() + ">", indents);
        indents++;

        for (std::shared_ptr<ASTNode> const& child : get_children()) {
            child->print(indents);
        }

        indents--;
        write_line("</" + get_token().get_token() + ">", indents);
    }
    
    void codegen(std::ofstream& out, SymbolTable& symbol_table) override {
        
    }
};
//

//
/* Expressions */

//
class ExpressionNode : public ASTNode {
private:
    Token value;
    std::shared_ptr<ExpressionNode> left, right;
public:
    ExpressionNode() : ASTNode() {}

    ExpressionNode(Token t) : value(t), ASTNode(t) {}
    
    ExpressionNode(Token t, std::shared_ptr<ExpressionNode> left_ptr, std::shared_ptr<ExpressionNode> right_ptr) : value(t), ASTNode(t) {
        std::shared_ptr<ASTNode> left_child = std::make_shared<ExpressionNode>(*left_ptr);
        std::shared_ptr<ASTNode> right_child = std::make_shared<ExpressionNode>(*right_ptr);
        left = left_ptr;
        right = right_ptr;
        add_child(left_child);
        add_child(right_child);
    }
    
    ExpressionNode& operator=(ExpressionNode other) {
        value = other.get_value();
        left = other.get_left();
        right = other.get_right();
        
        return *this;
    }

    Token get_value() {
        return value;
    }

    void set_value(Token value) {
        this->value = value;
    }

    std::shared_ptr<ExpressionNode> get_left() {
        return left;
    }

    void set_left(ExpressionNode left) {
        this->left = std::make_shared<ExpressionNode>(left);
    }

    std::shared_ptr<ExpressionNode> get_right() {
        return right;
    }

    void set_right(ExpressionNode right) {
        this->right = std::make_shared<ExpressionNode>(right);
    }

    virtual void print(int indents=0) override {
        write_line("<expr_node>", indents);
        indents++;
        
        if (left != nullptr) {
            left->print(indents);
        }
        
        write_line(value.get_token(), indents);
        
        if (right != nullptr) {
            right->print(indents);
        }
        
        indents--;
        write_line("</expr_node>", indents);
    }
    
    virtual void codegen(std::ofstream& out, SymbolTable& symbol_table) override {
        std::unordered_map<std::string, std::string> fmap_binary = { {"+", "fadd"}, {"-", "fsub"}, {"*", "fmult"}, {"/", "fdiv"} };
        std::unordered_map<std::string, std::string> fmap_unary = { {"-", "fneg"} };
        int f1 = -1, f2 = -1;
        
        if (left != nullptr) {
            f1 = 1;
            left->codegen(out, symbol_table);
        }
        
        if (right != nullptr) {
            f2 = 1;
            right->codegen(out, symbol_table);
        }
        
        if (f1 == 1 && f2 == 1) {
            if (fmap_binary.find(value.get_token()) != fmap_binary.end()) {
                out << fmap_binary.at(value.get_token()) << std::endl;
            } /* else if (op in tmap_binary) { out << tmap_binary.at(value.get_token()) << std::endl; } */
        } else if (f1 == 1 || f2 == 1) {
            if (fmap_unary.find(value.get_token()) != fmap_unary.end()) {
                out << fmap_unary.at(value.get_token()) << std::endl;
            } /* else if (op in tmap_unary) { out << tmap_unary.at(value.get_token()) << std::endl; } */
        }

    }
};

class TensorNode : public ExpressionNode {
private:
    int index;
    std::shared_ptr<TensorNode> first_child;
    std::shared_ptr<TensorNode> next_sibling;
public:
    TensorNode() : ExpressionNode(Token("tensor")) {
        first_child = std::shared_ptr<TensorNode>();
        next_sibling = std::shared_ptr<TensorNode>();
    }
    
    TensorNode(std::string number, DataType dtype) : ExpressionNode(Token(number, dtype_to_ttype.at(dtype))) {
        first_child = nullptr;
        next_sibling = nullptr;
    }
    
    int get_index() { return index; }
    
    void set_index(int index) { this->index = index; }
    
    std::shared_ptr<TensorNode> get_first_child() { return first_child; }
    
    void set_first_child(std::shared_ptr<TensorNode> first_child) { this->first_child = first_child; }
    
    std::shared_ptr<TensorNode> get_next_sibling() { return next_sibling; }
    
    void set_next_sibling(std::shared_ptr<TensorNode> next_sibling) { this->next_sibling = next_sibling; };
    
    bool is_leaf() {
        return false;
    }
    
    void codegen(std::ofstream& out, SymbolTable& symbol_table) override {
        VMWriter::write_push(out, "constant", index);
        first_child->codegen(out, symbol_table);
        next_sibling->codegen(out, symbol_table);
    }
    
    void print(int indents=0) override {
        write_line("<tensor>", indents);
        indents++;
        
        first_child->print(indents=indents);
        next_sibling->print(indents=indents);
        
        indents--;
        write_line("</tensor>", indents);
    }
};

//class TensorNode : public ExpressionNode {
//private:
//    bool tensor_structure;
//    std::unordered_map<int, std::shared_ptr<TensorNode>> tensor_tree;
//public:
//    TensorNode() : ExpressionNode(Token("tensor")) {
//        tensor_structure = true;
//    }
//
//    TensorNode(std::string number) : ExpressionNode(Token(number)) {
//        tensor_structure = false;
//    }
//
//    TensorNode(std::string number, DataType dtype) : ExpressionNode(Token(number, ddtype_to_ttype.at(dtype))) {
//        tensor_structure = false;
//    }
//
//    bool is_tensor_structure() {
//        return tensor_structure;
//    }
//
//    std::unordered_map<int, std::shared_ptr<TensorNode>> get_tensor_tree() {
//        return tensor_tree;
//    }
//
//    std::shared_ptr<TensorNode> get_subtensor(int index) {
//        return tensor_tree.at(index);
//    }
//
//    void add_index(int index) {
//        add_value(index, std::make_shared<TensorNode>(TensorNode()));
//    }
//
//    void add_value(int index, std::shared_ptr<TensorNode> value) {
//        tensor_tree.insert({index, value});
//    }
//
//    int height(TensorNode* tensor_node, int max_height=0) {
//        if (tensor_node == nullptr || tensor_node->get_token().get_token() != "tensor") {
//            return 0;
//        } else {
//            for (auto const& entry : tensor_node->get_tensor_tree()) {
//                int curr_height = height(&*entry.second);
//
//                if (curr_height > max_height) { max_height = curr_height; }
//            }
//        }
//
//        return max_height + 1;
//    }
//
//    void print(int indents=0) override {
//        write_line("<tensor_node>", indents);
//        indents++;
//
//        for (auto const& entry : tensor_tree) {
//            write_line("idx: " + std::to_string(entry.first), indents);
//            entry.second->print(indents);
//        }
//
//        indents--;
//        write_line("</tensor_node>", indents);
//    }
//
//    void codegen(std::ofstream& out, SymbolTable& symbol_table) override {
//        VMWriter::write_pop(out, "pointer", 0);
//        VMWriter::write_push(out, "pointer", 0);
//        VMWriter::write_newline(out);
//
//        codegen_helper(out, symbol_table);
//    }
//
//    void codegen_helper(std::ofstream& out, SymbolTable& symbol_table, bool first_pass=true, int this_index=0) {
//        for (auto const& entry : tensor_tree) {
//            if (entry.second->get_token().get_token() == "tensor") {
//                entry.second->codegen_helper(out, symbol_table, first_pass=false, this_index=this_index);
////                out << "---------" << std::endl;
//            } else {
//                VMWriter::write_push(out, "pointer", 0);
//                entry.second->codegen(out, symbol_table);
//                VMWriter::write_pop(out, "this", this_index);
//                this_index += 1;
//                VMWriter::write_pop(out, "this", this_index);
//            }
//
//            VMWriter::write_push(out, "constant", entry.first);
//            this_index += 1;
//            VMWriter::write_pop(out, "this", this_index);
//            VMWriter::write_newline(out);
//        }
//    }
//
////    void codegen_helper(std::ofstream& out, SymbolTable& symbol_table, bool first_pass=true, int this_index=0) {
////        unsigned long malloc_size = first_pass ? tensor_tree[0]->get_tensor_tree().size() + 1 : tensor_tree.size();
////        VMWriter::write_malloc(out, malloc_size);
////
////        if (!first_pass) {
////            VMWriter::write_pop(out, "this", this_index);
////            VMWriter::write_push(out, "this", this_index);
////        }
////
////        this_index = 0;
////        int i = 0;
////
////        for (auto const& entry : tensor_tree) {
////            if (i > 0) {
////                VMWriter::write_malloc(out, malloc_size);
////            }
////
////            VMWriter::write_pop(out, "pointer", 0);
////            VMWriter::write_push(out, "constant", entry.first);
////            VMWriter::write_pop(out, "this", this_index);
////            this_index += 1;
////
////            for (int j = 0; j < entry.second->get_tensor_tree().size(); j++) {
////                VMWriter::write_push(out, "constant", 0);
////                VMWriter::write_pop(out, "this", this_index + j);
////            }
////
//////            for (int j = 1; j < entry.second->get_tensor_tree().size(); j++) {
//////                VMWriter::write_push(out, "this", this_index + j);
//////            }
////
////
////            if (entry.second->get_token().get_token() == "tensor") {
////                entry.second->codegen_helper(out, symbol_table, first_pass=false, this_index=this_index);
////            } else {
////                VMWriter::write_push(out, "constant", 0);
////                VMWriter::write_pop(out, "this", 1);
////                VMWriter::write_malloc(out, 1);
////                VMWriter::write_pop(out, "this", 1);
////                VMWriter::write_push(out, "this", 1);
////                VMWriter::write_pop(out, "pointer", 0);
////                entry.second->codegen(out, symbol_table);
////                VMWriter::write_pop(out, "this", 0);
////            }
////
////            i++;
////        }
////    }
//
////    void codegen_helper(std::ofstream& out, SymbolTable& symbol_table, int this_index=0) {
////        out << "push " << "constant " << 3 << std::endl;
////        out << "call Memory.alloc 1" << std::endl;
////        out << "pop " << "pointer " << 0 << std::endl;
////
////        for (auto const& entry : tensor_tree) {
////            for (int i = 0; i < 3; i++) {
////                out << "push " << "constant " << entry.first << std::endl;
////                out << "pop " << "this " << i << std::endl;
////            }
////
////            this_index += 1;
////
//////            out << "pop\t\t" << (char) VirtualSegment::POINTER << 0 << std::endl;
//////            std::cout << entry.first << std::endl;
////
////            for (int i = 0; i < entry.second->tensor_tree.size(); i++) {
////                out << "push " << "pointer " << 0 << std::endl;
////                out << "pop " << "this " << this_index << std::endl;
////                out << "push " << "constant " << entry.second->tensor_tree.size() << std::endl;
////                out << "call Memory.alloc 1" << std::endl;
//////                out << "pop\t\t" << "this\t\t" << this_index << std::endl;
//////                out << "push\t" << "this\t\t" << this_index << std::endl;
////                out << "pop " << "pointer " << 0 << std::endl;
////
////                for (int j = 0; j < entry.second->tensor_tree.size(); j++) {
////                    out << "push " << "constant " << 0 << std::endl;
////                    out << "pop " << "this " << j << std::endl;
////                }
////
////                this_index += 1;
////            }
////
////            if (entry.second->get_token().get_token() == "tensor") {
////                entry.second->codegen_helper(out, symbol_table, this_index=this_index);
////            } else {
////                entry.second->codegen(out, symbol_table);
////            }
////
//////            entry.second->codegen(out, symbol_table);
////        }
////    }
//
////    void codegen(std::ofstream& out, SymbolTable& symbol_table) override {
////        int h = height(this);
////
////        for (int i = 0; i <= h; i++) {
////            codegen_curr_level(this, i, out);
////        }
////    }
////
////    void codegen_curr_level(TensorNode* tensor_node, int level, std::ofstream& out) {
////        if (tensor_node == nullptr) {
////            return;
////        } else if (level == 0) {
////            out << "push\t" << std::endl;
////        } else if (level > 0) {
////            int i = 0;
////
////            for (auto const& entry : tensor_node->get_tensor_tree()) {
////                if (i > 0) {
////                    std::cout << entry.first << std::endl;
////                    codegen_curr_level(&*entry.second, level - 1, out);
////                }
////
////                i++;
////            }
////        }
////    }
//};

class ScalarNode : public TensorNode {
private:
    double number;
    DataType dtype;
public:
    ScalarNode(std::string number, DataType dtype) : TensorNode(number, dtype) {
        this->number = std::stod(number);
        this->dtype = dtype;
    }

    double get_number() {
        return number;
    }

    DataType get_dtype() {
        return dtype;
    }
    
    bool is_leaf() {
        return true;
    }

    void print(int indents=0) override {
        std::ostringstream os;
        os << number;
        write_line(os.str(), indents);
    }
    
    void codegen(std::ofstream& out, SymbolTable& symbol_table) override {
        VMWriter::write_push(out, "constant", number);
    }
};

class IndentifierNode : public ExpressionNode {
private:
    std::string name;
public:
    IndentifierNode(std::string n) : name(n), ExpressionNode(Token(n)) {
//        std::cout << n << std::endl;
    }
    
    std::string get_name() {
        return name;
    }
    
    void set_name(std::string name) {
        this->name = name;
    }
    
    void print(int indents=0) override {}
    
    void codegen(std::ofstream& out, SymbolTable& symbol_table) override {
        VarKind kind = symbol_table.kind_of(name);
        
        if (kind == VarKind::NONE) {
            IllegalIdentifierError(-1);
        }
        
        VMWriter::write_push(out, vkind_to_vsegment.at(kind), symbol_table.index_of(name));
    }
};

class VarDecNode : public ASTNode {
private:
    std::string name;
    std::string type;
    VarKind kind;
    std::shared_ptr<ExpressionNode> rhs;
public:
    VarDecNode(std::string name, std::string type, VarKind kind, std::shared_ptr<ExpressionNode> right) : ASTNode(Token("var_dec")) {
        this->name = name;
        this->type = type;
        this->kind = kind;
        rhs = right;
    }

    std::string get_name() {
        return name;
    }

    std::string get_type() {
        return type;
    }

    VarKind get_kind() {
        return kind;
    }

    std::shared_ptr<ExpressionNode> get_rhs() {
        return rhs;
    }

    void print(int indents=0) override {
        write_line("<var_dec>", indents);
        indents++;
        
        write_line(name + "{" + type + "}: ", indents);
        rhs->print(indents);
        
        indents--;
        write_line("</var_dec>", indents);
    }
    
    void codegen(std::ofstream& out, SymbolTable& symbol_table) override {
        rhs->codegen(out, symbol_table);
        symbol_table.define(name, type, kind);
        
        if (rhs != nullptr) {
            VMWriter::write_pop(out, vkind_to_vsegment.at(kind), symbol_table.index_of(name));
        }
    }
};
