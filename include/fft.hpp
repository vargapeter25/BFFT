/***
 * Blaschke FFT experimantal version
*/

#ifndef FFT__HPP
#define FFT__HPP

#include <vector>
#include <random>
#include <iterator>
#include <numbers>

#include "function_system.h"
#include "utils.hpp"
#include "interpolation.hpp"
#include "mpl.hpp"

namespace bfft{

class BlaschkeFFT{
public:
    typedef BlaschkeFunction::value_type value_type;

    enum ResizeType { RESIZE, LINEAR_INTERPOLATION };

    BlaschkeFFT() {}
    BlaschkeFFT(const FunctionSystem& function_system) : m_function_system(function_system) {}

    template<mpl::InputIteratorType InputIterator>
    std::vector<value_type> fft(InputIterator first, InputIterator last, ResizeType resize_type = ResizeType::RESIZE) const ;
    template<mpl::ContainerType Container>
    std::vector<value_type> fft(const Container& container, ResizeType resize_type = ResizeType::RESIZE) const { return fft(container.begin(), container.end(), resize_type); }
    template<mpl::InputIteratorType InputIterator>
    std::vector<value_type> ifft(InputIterator first, InputIterator last, size_t out_n = 0, ResizeType resize_type = ResizeType::RESIZE) const;
    template<mpl::ContainerType Container>
    std::vector<value_type> ifft(const Container& container, size_t out_n = 0, ResizeType resize_type = ResizeType::RESIZE) const { return ifft(container.begin(), container.end(), out_n, resize_type); }

    FunctionSystem& function_system() { return m_function_system; }
    const FunctionSystem& function_system() const { return m_function_system; }

    template<mpl::InputIteratorType InputIterator>
    std::vector<value_type> resize_input(InputIterator first, InputIterator last, size_t n, ResizeType resize_type) const;
    template<mpl::InputIteratorType InputIterator>
    std::vector<value_type> resize_output(InputIterator first, InputIterator last, size_t n, ResizeType resize_type) const;

    template<mpl::InputIteratorType InputIterator>
    std::vector<value_type> resize_vector(InputIterator first, InputIterator last, size_t n) const;
    
    template<mpl::InputIteratorType InputIterator>
    std::vector<value_type> resize_input_linear_interpolation(InputIterator first, InputIterator last, size_t n) const;
    template<mpl::InputIteratorType InputIterator>
    std::vector<value_type> resize_output_linear_interpolation(InputIterator first, InputIterator last, size_t n) const;

private:
    FunctionSystem m_function_system;
};

template<mpl::InputIteratorType InputIterator>
std::vector<BlaschkeFFT::value_type> BlaschkeFFT::fft(InputIterator first, InputIterator last, ResizeType resize_type) const {
    int input_size = std::distance(first, last);
    ASSERT((0 < input_size), "Input size must be at least 1!");

    size_t n = 1, n_log = 0;
    while(n < static_cast<size_t>(input_size)) {
        n <<= 1;
        n_log++;
    }
    std::vector<BlaschkeFFT::value_type> c = resize_input(first, last, n, resize_type);
    const std::vector<std::vector<BlaschkeFFT::value_type>>& base_points = m_function_system.base_points_lvl(n_log, 1);

    for(size_t phase = 0; (1ULL<<phase) < n; phase++){
        size_t part_width = n / (1ULL<<phase);
        for(size_t part = 0; part < n / part_width; part++){
            for(size_t butterfly = 0; butterfly < part_width / 2; butterfly++){
                size_t i = part_width * part + butterfly;
                size_t j = i + part_width / 2;

                Complex tmp = c[i];
                c[i] = (tmp + c[j]) * 0.5;
                c[j] = Complex::conj_mult(tmp - c[j], base_points[n_log - phase][butterfly]) * 0.5;
            }
        }
    }

    std::vector<size_t> reverse_bit_order(n);
    for(size_t i = 1; i < n; i++){
        reverse_bit_order[i] = (reverse_bit_order[i / 2] >> 1);
        if(i&1) reverse_bit_order[i] |= n >> 1;
    }

    for(size_t i = 0; i < n; i++){
        if(i < reverse_bit_order[i]) std::swap(c[i], c[reverse_bit_order[i]]);
    }

    return c;
}

template<mpl::InputIteratorType InputIterator>
std::vector<BlaschkeFFT::value_type> BlaschkeFFT::ifft(InputIterator first, InputIterator last, size_t out_n, ResizeType resize_type) const {
    int input_size = std::distance(first, last);
    ASSERT((0 < input_size), "Input size must be at least 1!");

    size_t n = 1, n_log = 0;
    while(n < static_cast<size_t>(input_size)) {
        n <<= 1;
        n_log++;
    }

    if(out_n == 0) out_n = n;

    std::vector<BlaschkeFFT::value_type> c(n);
    std::copy(first, last, c.begin());

    const std::vector<std::vector<BlaschkeFFT::value_type>>& base_points = m_function_system.base_points_lvl(n_log, 1);

    std::vector<size_t> reverse_bit_order(n);
    for(size_t i = 1; i < n; i++){
        reverse_bit_order[i] = (reverse_bit_order[i / 2] >> 1);
        if(i&1) reverse_bit_order[i] |= n >> 1;
    }

    for(size_t i = 0; i < n; i++){
        if(i < reverse_bit_order[i]) std::swap(c[i], c[reverse_bit_order[i]]);
    }

    for(size_t phase = 1; (1ul<<phase) <= n; phase++){
        size_t part_width = (1ul<<phase);
        size_t parts = n / part_width;
        for(size_t part = 0; part < parts; part++){
            for(size_t butterfly = 0; butterfly < part_width / 2; butterfly++){
                size_t i = part_width * part + butterfly;
                size_t j = i + part_width / 2;

                Complex tmp = c[j] * base_points[phase][butterfly];
                c[j] = c[i] - tmp;
                c[i] += tmp;
            }
        }
    }

    return resize_output(c.begin(), c.end(), out_n, resize_type);
}

template<mpl::InputIteratorType InputIterator>
std::vector<BlaschkeFFT::value_type> BlaschkeFFT::resize_input(InputIterator first, InputIterator last, size_t n, ResizeType resize_type) const {
    switch (resize_type)
    {
    case ResizeType::RESIZE:
        return resize_vector(first, last, n);
        break;
    case ResizeType::LINEAR_INTERPOLATION:
        return resize_input_linear_interpolation(first, last, n);
        break;
    default:
        return resize_vector(first, last, n);
        break;
    }
}

template<mpl::InputIteratorType InputIterator>
std::vector<BlaschkeFFT::value_type> BlaschkeFFT::resize_vector(InputIterator first, InputIterator last, size_t n) const {
    std::vector<BlaschkeFFT::value_type> result(n);
    size_t source_size = std::distance(first, last);
    if (source_size < n) {
        std::copy(first, last, result.begin());
    } else {
        std::copy(first, std::next(first, n), result.begin());
    }
    return result;    
}

template<mpl::InputIteratorType InputIterator>
std::vector<BlaschkeFFT::value_type> BlaschkeFFT::resize_input_linear_interpolation(InputIterator first, InputIterator last, size_t n) const {
    auto interpolation_points = create_uniform_interpolation_points<double>(std::vector<value_type>(first, last));
    auto sample_points = m_function_system.sample_points(ceil_log2(n), Complex(1)); // create_sample_points<double>(m_function_system, ceil_log2(n));
    return linear_interpolation_vector(interpolation_points, sample_points);
}

template<mpl::InputIteratorType InputIterator>
std::vector<BlaschkeFFT::value_type> BlaschkeFFT::resize_output(InputIterator first, InputIterator last, size_t n, ResizeType resize_type) const {
    switch (resize_type)
    {
    case ResizeType::RESIZE:
        return resize_vector(first, last, n);
        break;
    case ResizeType::LINEAR_INTERPOLATION:
        return resize_output_linear_interpolation(first, last, n);
        break;
    default:
        return resize_vector(first, last, n);
        break;
    }
}


template<mpl::InputIteratorType InputIterator>
std::vector<BlaschkeFFT::value_type> BlaschkeFFT::resize_output_linear_interpolation(InputIterator first, InputIterator last, size_t n) const {    
    auto interpolation_points = create_func_base_interpolation_points<double>(m_function_system, std::vector<BlaschkeFFT::value_type>(first, last));
    auto sample_points = create_uniform_sample_points<double>(n);
    return linear_interpolation_vector(interpolation_points, sample_points);
}

}

#endif //FFT__HPP