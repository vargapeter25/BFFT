#ifndef INTERPOLATION__HPP
#define INTERPOLATION__HPP

#include "function_system.h"
#include <vector>
#include <numbers>

namespace bfft{

template <typename T, typename U>
struct InterpolationPoint {
    T pos;
    U val;
};

template<typename T, typename U>
inline U linear_interpolation(const InterpolationPoint<T, U>& p0, const InterpolationPoint<T, U>& p1, const T& x) {
    if(p0.pos == p1.pos) return p0.val;
    return p0.val + (p1.val - p0.val) * (x - p0.pos) / (p1.pos - p0.pos);
}

template<typename T, typename U>
std::vector<U> linear_interpolation_vector(const std::vector<InterpolationPoint<T, U>>& base_points, const std::vector<T>& sample_pos) {
    ASSERT(!base_points.empty(), "Base points must contain at least 1 point!");
    std::vector<U> sample_val(sample_pos.size());

    int j = 0, base_size = static_cast<int>(base_points.size());
    for(size_t i = 0; i < sample_pos.size(); i++){
        while(j < base_size && base_points[j].pos < sample_pos[i]){
            ++j;
        }
        int idx_prev = (j > 0         ? j - 1 : 0);
        int idx_next = (j < base_size ? j : base_size - 1);
        sample_val[i] = linear_interpolation(base_points[idx_prev], base_points[idx_next], sample_pos[i]);
    }
    return sample_val;
}

template<typename T, typename U>
std::vector<InterpolationPoint<T, U>> create_interpolation_points(const std::vector<T>& pos, const std::vector<U>& val) { 
    ASSERT(pos.size() == val.size(), "Points size and value size must be the same!");
    std::vector<InterpolationPoint<T, U>> interpolation_points(pos.size());
    for(size_t i = 0; i < interpolation_points.size(); i++){
        interpolation_points[i].pos = pos[i];
        interpolation_points[i].val = val[i];
    }
    return interpolation_points;
}

template<typename T>
std::vector<T> create_uniform_sample_points(size_t n){
    std::vector<T> result(n);
    for(size_t i = 0; i < n; i++){
        result[i] = static_cast<T>(1) / static_cast<T>(n) * static_cast<T>(i);
    }
    return result;
}

template<typename T, typename U>
std::vector<InterpolationPoint<T, U>> create_uniform_interpolation_points(const std::vector<U>& v){
    return create_interpolation_points(create_uniform_sample_points<double>(v.size()), v);
}

template<typename T>
std::vector<T> create_sample_points(const FunctionSystem& func_sys, size_t n, Complex val = Complex(1)) {
    const std::vector<BlaschkeFunction::value_type>& base_points = func_sys.base_points(n, val);
    std::vector<T> pos(base_points.size());
    for(size_t i = 0; i < pos.size(); i++){
        pos[i] = std::acos(std::clamp(base_points[i].real, -1.0, 1.0));
        if(base_points[i].imag < 0.0) pos[i] = std::numbers::pi * 2 - pos[i];
        pos[i] /= std::numbers::pi * 2;
    }

    // make non cyclic
    size_t idx = std::distance(pos.begin(), std::min_element(pos.begin(), pos.end()));
    for(size_t i = 0; i < idx; i++) pos[i] -= 1.0;
    for(size_t i = 0; i+1 < pos.size(); i++) if(pos[i+1] < pos[i]) pos[i+1] += 1.0;
    return pos;
}

template<typename T, typename U>
std::vector<InterpolationPoint<T, U>> create_func_base_interpolation_points(const FunctionSystem& func_sys, const std::vector<U>& val) { 
    ASSERT(ceil_pow2(val.size()) == val.size(), "Number of values must be a power of two!");
    return create_interpolation_points(func_sys.sample_points(ceil_log2(val.size()), Complex(1)), val);
    // return create_interpolation_points(create_sample_points<T>(func_sys, ceil_log2(val.size())), val);
}

}

#endif //INTERPOLATION__HPP