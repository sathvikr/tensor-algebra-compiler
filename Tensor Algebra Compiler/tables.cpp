//
//  tables.cpp
//  Tensor Algebra Compiler
//

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <complex>
#include <cmath>
#include <vector>
#include <sstream>

using namespace std::complex_literals;

struct vd_hash {
    size_t operator()(const std::vector<double>& v) const {
        size_t seed = v.size();
        for(auto &d : v) { seed ^= (size_t) d + 0x9e3779b9 + (seed << 6) + (seed >> 2); }
        return seed;
    }
};

class Table {
private:
    std::unordered_map<std::vector<double>, std::vector<std::complex<double>>, vd_hash> m;
    
    static std::complex<double> theta2(std::vector<double>& u2) {
        auto c = 1i * u2[2];
        return 1i * log(-(u2[0] - c) / (u2[0] + c));
    }

    static std::complex<double> alpha2(std::vector<double>& u2) {
        return 1i * log(-u2[2] - u2[0] * 1i);
    }
    
    static std::complex<double> beta2(std::vector<double>& u2) {
        return 1i * log(u2[3] + 1i * u2[1]);
    }
public:
    Table() {
        this->m = std::unordered_map<std::vector<double>, std::vector<std::complex<double>>, vd_hash>();
    }

    void init_u22angle(double prec) {
        for (double i = 0; i < 2 * M_PI; i += prec) {
            for (double j = 0; j < 2 * M_PI; j += prec) {
                for (double k = 0; k < 2 * M_PI; k += prec) {
                    for (double l = 0; l < 2 * M_PI; l += prec) {
                        std::vector<double> u2 = {i, j, k, l};
                        std::vector<std::complex<double>> angles = {theta2(u2), alpha2(u2), beta2(u2)};
                        m.insert({u2, angles});
                    }
                }
            }
        }
    }
    
    std::unordered_map<std::vector<double>, std::vector<std::complex<double>>, vd_hash> get_u22angle() {
        return m;
    }
    
    void read_u22angle(std::ifstream& fin) {
        char buffer[1024] = {};
        fin.getline(buffer, 1024);
        while (fin.getline(buffer, 1024)) {
            std::vector<double> u2 = {std::stod(strtok(buffer, ",")), std::stod(strtok(NULL, ",")), std::stod(strtok(NULL, ",")), std::stod(strtok(NULL, ","))};
            char* theta2_tok = strtok(NULL, "\""); strtok(NULL, "\"");
            char* alpha2_tok = strtok(NULL, "\""); strtok(NULL, "\"");
            char* beta2_tok = strtok(NULL, "\"");
            std::complex<double> theta2; std::istringstream(theta2_tok) >> theta2;
            std::complex<double> alpha2; std::istringstream(alpha2_tok) >> alpha2;
            std::complex<double> beta2; std::istringstream(beta2_tok) >> beta2;
            std::vector<std::complex<double>> angles = {theta2, alpha2, beta2};
            m.insert({u2, angles});
        }
    }

    void write_u22angle(std::ofstream& fout) {
        std::cout << m.size() << std::endl;
        fout << "u11,u21,u12,u22,theta(2),alpha(2),beta(2)\n";
        for (auto pair : m) {
            for (double d : pair.first) { fout << d << ","; }
            for (size_t i = 0; i < pair.second.size(); i++) {
                fout << "\"" << pair.second[i] << "\"";
                if (i < pair.second.size() - 1) { fout << ","; }
            }
            fout << "\n";
        }
        fout.close();
    }
    
    void print_u22angle() {
        for (auto pair : m) {
            for (double d : pair.first) { std::cout << d << ","; }
            for (size_t i = 0; i < pair.second.size(); i++) {
                std::cout << "\"" << pair.second[i] << "\"";
                if (i < pair.second.size() - 1) { std::cout << ","; }
            }
            std::cout << "\n";
        }
    }
    
    std::vector<std::complex<double>> lookup_u2_angles(std::vector<double>& u2) {
        return m.at(u2);
    }
    
//    std::vector<std::vector<double>> J_u22u4() {
//
//    }
};
