#ifndef FUNCTION_SYSTEM__H
#define FUNCTION_SYSTEM__H

#include <vector>
#include "complex.h"
#include <algorithm>

namespace bfft{

class BlaschkeFunction{
public:
    using value_type = Complex;

    BlaschkeFunction() : m_param(0) {}
    BlaschkeFunction(const value_type& param) : m_param(param) {}

    value_type operator()(const value_type& x) const;
    std::pair<value_type, value_type> get_roots(const value_type& x) const;

    value_type get_param() const { return m_param; }

private:
    value_type m_param;
};

class FunctionSystem{
public:
    FunctionSystem(size_t n = 1, const BlaschkeFunction::value_type& default_param = Complex(0.0))
                        : m_func_vec(0), m_default(default_param), m_cache_ok(false), m_sample_cache_ok(false) { resize(n); }
    FunctionSystem(const std::vector<BlaschkeFunction::value_type>& params,
                   const BlaschkeFunction::value_type& default_param = Complex(0.0))
                        : m_func_vec(0), m_default(default_param), m_cache_ok(false), m_sample_cache_ok(false) { set_functions(params); }

    void set_functions(const std::vector<BlaschkeFunction::value_type>& params);

    void set_function(size_t i, const BlaschkeFunction::value_type& param);

    void resize(size_t n) { m_cache_ok = false; m_func_vec.resize(n); }

    inline const BlaschkeFunction& at(size_t i) const { return i < size() ? m_func_vec[i] : m_default; }
    BlaschkeFunction& get_default() { m_cache_ok = false; return m_default; }

    inline size_t size() const { return m_func_vec.size(); }

    std::vector<BlaschkeFunction>& get_functions() { m_cache_ok = false; return m_func_vec; }
    std::vector<BlaschkeFunction::value_type> get_function_params() const;

    const std::vector<BlaschkeFunction::value_type>& base_points(size_t, const BlaschkeFunction::value_type&) const;
    const std::vector<std::vector<BlaschkeFunction::value_type>>& base_points_lvl(size_t, const BlaschkeFunction::value_type&) const;
    const std::vector<double>& sample_points(size_t, const BlaschkeFunction::value_type&) const;

    BlaschkeFunction::value_type eval(size_t n, BlaschkeFunction::value_type x) const;
    BlaschkeFunction::value_type eval_any(size_t n, BlaschkeFunction::value_type x) const;

private:
    std::vector<BlaschkeFunction> m_func_vec;
    BlaschkeFunction m_default;
    mutable std::vector<std::vector<BlaschkeFunction::value_type>> m_cached_base_points;
    mutable size_t m_cached_lvl{};
    mutable BlaschkeFunction::value_type m_cached_param;
    mutable bool m_cache_ok;
    mutable std::vector<double> m_cached_samples;
    mutable bool m_sample_cache_ok;

    bool calc_base_points(size_t, const BlaschkeFunction::value_type&) const;
};

}

#endif //FUNCTION_SYSTEM__H