#ifndef UTILS__HPP
#define UTILS__HPP

#include <cstdlib>
#include <utility>
#include <numbers>
#include <iterator>
#include <cassert>
#include <bit>

#include "mpl.hpp"
#include "complex.h"

namespace bfft{

inline size_t ceil_pow2(size_t n) {
    return std::bit_ceil(n);
}

inline size_t ceil_log2(size_t n) {
    return n <= 1 ? 0 : std::bit_width(n - 1);
}

template<mpl::InputIteratorType InputIterator1, mpl::InputIteratorType InputIterator2>
double mean_squared_error(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2){
    ASSERT((std::distance(first1, last1) == std::distance(first2, last2)), "The two data size must be equal!");
    size_t input_size = std::distance(first1, last1);
    double result = 0;
    while(first1 != last1 && first2 != last2){
        result += Complex::abs(*first1 - *first2);
        ++first1;
        ++first2;
    }
    return result / input_size;
}
    
template<mpl::ContainerType Container1, mpl::ContainerType Container2>
double mean_squared_error(const Container1& c1, const Container2& c2){
    return mean_squared_error(c1.begin(), c1.end(), c2.begin(), c2.end());
}

}

#endif //UTILS__HPP