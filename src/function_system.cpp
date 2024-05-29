#include "../include/function_system.h"
#include "../include/interpolation.hpp"
#include "../include/mpl.hpp"

using namespace bfft;

BlaschkeFunction::value_type BlaschkeFunction::operator()(const value_type& x) const{
     return (x*x - m_param*m_param) / (-Complex::conj(m_param*m_param) * x * x + 1.0);
}

std::pair<BlaschkeFunction::value_type, BlaschkeFunction::value_type> BlaschkeFunction::get_roots(const value_type& x) const {
    value_type root = Complex::sqrt((m_param * m_param + x) / (Complex::conj_mult(x, m_param * m_param) + 1.0));
    return root.real >= 0 ? std::make_pair(root, -root) : std::make_pair(-root, root); 
}

void FunctionSystem::set_functions(const std::vector<BlaschkeFunction::value_type>& params) {
    m_cache_ok = false;
    m_func_vec.resize(params.size());
    for(size_t i = 0; i < params.size(); i++){
        m_func_vec[i] = BlaschkeFunction(params[i]);
    }
}

void FunctionSystem::set_function(size_t i, const BlaschkeFunction::value_type& param) {
    m_cache_ok = false;
    if(size() < i + 1) resize(i+1);
    m_func_vec[i] = BlaschkeFunction(param);
}

const std::vector<BlaschkeFunction::value_type>& FunctionSystem::base_points(size_t __n, const BlaschkeFunction::value_type& __val = Complex(1)) const {
    calc_base_points(__n, __val);
    ASSERT(!m_cached_base_points.empty(), "no empty");
    ASSERT((1ul << __n) == m_cached_base_points.back().size(), "correct size");
    return m_cached_base_points.back();
}

const std::vector<std::vector<BlaschkeFunction::value_type>>& FunctionSystem::base_points_lvl(size_t __n, const BlaschkeFunction::value_type& __val = Complex(1)) const {
    calc_base_points(__n, __val);
    ASSERT(__n + 1 == m_cached_base_points.size(), "ERROR");
    return m_cached_base_points;
}

bool FunctionSystem::calc_base_points(size_t __n, const BlaschkeFunction::value_type& __val) const {
    if(m_cache_ok && m_cached_lvl == __n && Complex::abs(__val - m_cached_param) < 1e-9){
        return false;
    }
    m_cached_base_points.resize(__n + 1);
    m_cached_base_points[0] = {__val};
    for(size_t i = __n; i > 0; i--){
        size_t root_cnt = 1ul<<(__n - i + 1);
        m_cached_base_points[__n - i + 1].resize(root_cnt);
        for(size_t j = 0; j < root_cnt/2; j++){
            auto curr_roots = at(i - 1).get_roots(m_cached_base_points[__n-i][j]);
            m_cached_base_points[__n - i + 1][j] = curr_roots.first;
            m_cached_base_points[__n - i + 1][j + root_cnt/2] = curr_roots.second;
        }
        for(auto x : m_cached_base_points[__n - i + 1]){
            ASSERT(std::abs(Complex::abs(x) - 1.0) < 1e-6, "Must be one length");
        }
        // Correct ordering.
        for(size_t j = 0; j < root_cnt/2 - 1; j++){
            if((m_cached_base_points[__n - i + 1][j] * Complex::conj(m_cached_base_points[__n - i + 1][j+1])).imag > 0){
                std::swap(m_cached_base_points[__n - i + 1][j+1], m_cached_base_points[__n - i + 1][root_cnt / 2 + j + 1]);
            }
        }
    }
    m_cache_ok = true;
    m_cached_lvl = __n;
    m_cached_param = __val;
    return true;
}

const std::vector<double>& FunctionSystem::sample_points(size_t n, const BlaschkeFunction::value_type& val) const{
    m_sample_cache_ok = !calc_base_points(n, val) && m_sample_cache_ok;
    if(!m_sample_cache_ok){
        m_cached_samples = create_sample_points<double>(*this, n, val);
        m_sample_cache_ok = true;
    }
    return m_cached_samples;
}

std::vector<BlaschkeFunction::value_type> FunctionSystem::get_function_params() const {
    std::vector<BlaschkeFunction::value_type> params(m_func_vec.size());
    for(size_t i = 0; i < m_func_vec.size(); i++) params[i] = m_func_vec[i].get_param();
    return params;
}

BlaschkeFunction::value_type FunctionSystem::eval(size_t __n, BlaschkeFunction::value_type __x) const {

    for(size_t i = 0; i < __n; i++){
        __x = at(i)(__x);
    }
    
    return __x;
}

BlaschkeFunction::value_type FunctionSystem::eval_any(size_t __n, BlaschkeFunction::value_type __x) const {
    BlaschkeFunction::value_type result = 1;

    for(size_t i = 0; (1ul<<i) <= __n; i++){
        if(__n&(1u<<i)) result *= __x;
        __x = at(i)(__x);
    }
    
    return result;
}