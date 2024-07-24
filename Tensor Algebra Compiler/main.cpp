//
//  main.cpp
//  Tensor Algebra Compiler
//
//

#include <iostream>
#include <fstream>
#include <string>

#include "code_generator.cpp"
#include "chip_sim.cpp"


void reconstruct(std::vector<double>& tree, int index=0, int indents=0) {
    if (index >= tree.size()) {
        return;
    }
    
    for (int i = 0; i < indents; i++) { std::cout << "\t"; }
    
    std::cout << tree[index] << std::endl;
    
    reconstruct(tree, 2 * index + 1, indents=indents + 1);
    reconstruct(tree, 2 * index + 2, indents=indents + 1);
}

int main(int argc, const char* argv[]) {
    std::string fname_base = "expression";
    std::string path_base = "/Users/sathvik.redrouthu/Desktop/Procyon/CS/Tensor Algebra Compiler/Tables/";
    std::string in_path = path_base + fname_base + ".apollo";
    std::string out_path_xml = path_base + fname_base + ".xml";
    std::string out_path_code = path_base + fname_base + ".ir";
//
    Tokenizer tokenizer = Tokenizer(in_path);
    Parser parser = Parser(in_path, out_path_xml);
    ProgramNode ast = parser.parse_compilation_unit();
    CodeGenerator code_generator = CodeGenerator(ast, out_path_code);

    code_generator.generate_code();
//    ast.print();
    
    
//    std::vector<double> tree{0, 0, 3.1, 1, 3.8, 3, 9.1, -1, 1, 0, 4.2};
//    reconstruct(tree);
    
    /*
                               0
                    /                  \
                0                               1
            /       \                         /
        3.1             1                   0
                    /       \           /
                3.8             3  4.2
                            /
                        9.1
     
     
     */
    
//    TensorNode tn = TensorNode();
//    tn.set_first_child(std::make_shared<TensorNode>(TensorNode()));
    
//    std::string u22angle_path = path_base + "u22angle.csv";
//    Chip tachyon = Chip();
//    Table t = Table();
////    std::ifstream fin = std::ifstream(u22angle_path);
////    t.read_u22angle(fin);
//    t.init_u22angle(0.5);
//    std::vector<double> u2 = {5, 5, 5, 5};
//    std::vector<std::complex<double>> u2_angles = t.lookup_u2_angles(u2);
//    std::vector<std::vector<std::complex<double>>> t_u2 = tachyon.get_u2(u2_angles);
//
//
//    for (int i = 0; i < t_u2.size(); i++) {
//        for (int j = 0; j < t_u2[i].size(); j++) { std::cout << t_u2[i][j] << ","; }
//        std::cout << "\n";
//    }
    
    return 0;
}
