//
//  chip_sim.cpp
//  Tensor Algebra Compiler
//
//

#include <stdio.h>
#include <vector>
#include <complex>

using namespace std::complex_literals;

class Chip {
public:
    Chip() {}
    
    // angles = [theta, alpha, beta]
    std::vector<std::vector<std::complex<double>>> get_u2(std::vector<std::complex<double>>& angles) {
        std::vector<std::complex<double>> expangs;
        for (std::complex<double> angle : angles) { expangs.push_back(std::exp(-1i * angle)); };
        return std::vector<std::vector<std::complex<double>>> {
            {0.5 * expangs[1] * (expangs[0] - 1.0), 0.5i * expangs[1] * (expangs[0] + 1.0)},
            {0.5i * expangs[2] * (expangs[0] + 1.0), 0.5 * expangs[2] * (1.0 - expangs[0])}
        };
    }
    
    std::complex<double> dot(std::vector<std::complex<double>>& a, std::vector<std::complex<double>>& b) {
        std::complex<double> res = 0;
        for (int i = 0; i < a.size(); i++) { res += a[i] * b[i]; }
        return res;
    }
    
    std::vector<std::complex<double>> mvm(std::vector<std::vector<std::complex<double>>>& m, std::vector<std::complex<double>>& v) {
        std::vector<std::complex<double>> res;
        for (int i = 0; i < m.size(); i++) { res.push_back(dot(m[i], v)); }
        return res;
    }
};
