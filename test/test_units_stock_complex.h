#include "test_common.h"

#ifndef TEST_UNITS_STOCK_COMPLEX_H
#define TEST_UNITS_STOCK_COMPLEX_H

template<typename F, size_t L>
void test_stock_complex_type() {
    std::string test_name = "stock complex<";
    test_name += (std::is_same<F,float>::value ? "float" : "double" );
    test_name += ", " + std::to_string(L) + ">";

    current_test<F, using_nompi> name(test_name);
    using Complex = heffte::stock::Complex<F,L>;
    using stdcomp = std::complex<F>;

    constexpr size_t vec_sz = (L == 1) ? L : L/2;
    std::vector<std::complex<F>> inp_left {};
    std::vector<std::complex<F>> inp_right {};
    std::vector<std::complex<F>> inp_last {};
    for(size_t i = 0; i < vec_sz; i++) {
        F i2 = (F) 2*i;
        inp_left.push_back(std::complex<F> {i2, i2 + 1.});
        inp_right.push_back(std::complex<F> {L-(i2+1.f), L-(i2+2.)});
        inp_left.push_back(std::complex<F> {2*i2, 2*i2 + 2.});
    }
    F scalar = 5.;
    constexpr size_t UNARY_OP_COUNT = 5;
    constexpr size_t BINARY_OP_COUNT = 4;
    constexpr size_t TERNARY_OP_COUNT = 2;
    std::array<std::function<stdcomp(stdcomp)>, UNARY_OP_COUNT> stlVecUnOps {
        [](stdcomp x) { return -x;                         },
        [](stdcomp x) { return conj(x);                    },
        [](stdcomp x) { return std::abs(x);                },
        [](stdcomp x) { return x*std::complex<F>{0.,  1.}; },
        [](stdcomp x) { return x*std::complex<F>{0., -1.}; }
    };
    std::array<std::function<stdcomp(stdcomp)>, UNARY_OP_COUNT> stockVecUnOps {
        [](Complex x) { return -x;              },
        [](Complex x) { return x.conjugate();   },
        [](Complex x) { return x.modulus();     },
        [](Complex x) { return x.__mul_i();     },
        [](Complex x) { return x.__mul_neg_i(); }
    };
    std::array<std::function<stdcomp(stdcomp, stdcomp)>, BINARY_OP_COUNT> stlVecBinOps {
        [](stdcomp x, stdcomp y) { return x + y; },
        [](stdcomp x, stdcomp y) { return x - y; },
        [](stdcomp x, stdcomp y) { return x * y; },
        [](stdcomp x, stdcomp y) { return x / y; }
    };
    std::array<std::function<Complex(Complex, Complex)>, BINARY_OP_COUNT> stockVecBinOps {
        [](Complex x, Complex y) { return x + y; },
        [](Complex x, Complex y) { return x - y; },
        [](Complex x, Complex y) { return x * y; },
        [](Complex x, Complex y) { return x / y; }
    };
    std::array<std::function<stdcomp(stdcomp, F)>, BINARY_OP_COUNT> stlScalarBinOps {
        [](stdcomp x, F y) { return x + y; },
        [](stdcomp x, F y) { return x - y; },
        [](stdcomp x, F y) { return x * y; },
        [](stdcomp x, F y) { return x / y; }
    };
    std::array<std::function<Complex(Complex, F)>, BINARY_OP_COUNT> stockScalarBinOps {
        [](Complex x, F y) { return x + y; },
        [](Complex x, F y) { return x - y; },
        [](Complex x, F y) { return x * y; },
        [](Complex x, F y) { return x / y; }
    };
    std::array<std::function<Complex(Complex, Complex, Complex)>, TERNARY_OP_COUNT> stockVecTriOps {
        [](Complex x, Complex y, Complex z){ return x.fmadd(y, z);},
        [](Complex x, Complex y, Complex z){ return x.fmsub(y, z);}
    };
    Complex comp_left {inp_left.data()};
    Complex comp_right {inp_right.data()};
    Complex comp_last {inp_last.data()};

    std::vector<std::complex<F>> ref_vec_un_out (vec_sz);
    std::vector<std::complex<F>> comp_vec_un_out (vec_sz);
    std::vector<std::complex<F>> ref_vec_bin_out (vec_sz);
    std::vector<std::complex<F>> comp_vec_bin_out (vec_sz);
    std::vector<std::complex<F>> ref_scalar_bin_out (vec_sz);
    std::vector<std::complex<F>> comp_scalar_bin_out (vec_sz);
    std::vector<std::complex<F>> ref_vec_tri_out (vec_sz);
    std::vector<std::complex<F>> comp_vec_tri_out (vec_sz);

    Complex comp_vec_out_i;
    Complex comp_scalar_out_i;

    for(size_t i = 0; i < UNARY_OP_COUNT; i++) {
        comp_vec_out_i = stockVecUnOps[i](comp_left, comp_right);
        for(size_t j = 0; j < vec_sz; j++) {
            ref_vec_out[j] = stlVecUnOps[i](inp_left[j]);
            comp_vec_out[j] = comp_vec_out_i[j];
        }
        sassert(approx(ref_vec_out, comp_vec_out));
    }

    for(size_t i = 0; i < BINARY_OP_COUNT; i++) {
        comp_vec_out_i = stockVecBinOps[i](comp_left, comp_right);
        comp_scalar_out_i = stockScalarBinOps[i](comp_left, scalar);
        for(size_t j = 0; j < vec_sz; j++) {
            ref_vec_out[j] = stlVecBinOps[i](inp_left[j], inp_right[j]);
            ref_scalar_out[j] = stlScalarBinOps[i](inp_left[j], scalar);
            comp_vec_out[j] = comp_vec_out_i[j];
            comp_scalar_out[j] = comp_scalar_out_i[j];
        }
        sassert(approx(ref_vec_out, comp_vec_out));
        sassert(approx(ref_scalar_out, comp_scalar_out));
    }

    for(size_t i = 0; i < TERNARY_OP_COUNT; i++) {
        comp_vec_out_i = stockVecTriOps[i](comp_left, comp_right, comp_last);
        for(size_t j = 0; j < vec_sz; j++) {
            ref_vec_out[j] = stlVecTriOps[i](inp_left[j], inp_right[j], inp_last[j]);
            comp_vec_out[j] = comp_vec_out_i[j];
        }
        sassert(approx(ref_vec_out, comp_vec_out));
    }

}

#endif // End TEST_UNITS_STOCK_COMPLEX_H