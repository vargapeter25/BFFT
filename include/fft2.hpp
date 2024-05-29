#ifndef FFT2__HPP
#define FFT2__HPP

#include "fft.hpp"
#include "matrix.hpp"
#include "utils.hpp"
#include "interpolation.hpp"

#include <vector>
#include <numbers>

namespace bfft{

class BlaschkeFFT2{
public:
    using value_type = matrix::Matrix<BlaschkeFunction::value_type>;
    using ResizeType = BlaschkeFFT::ResizeType;

    BlaschkeFFT2(size_t rows, size_t cols, const BlaschkeFFT& default_fft = BlaschkeFFT()) 
        : m_fft_rows(rows), m_fft_cols(cols), m_default_fft(default_fft) {} 
    BlaschkeFFT2(const std::vector<BlaschkeFFT>& fft_rows, const std::vector<BlaschkeFFT>& fft_cols, const BlaschkeFFT& default_fft = BlaschkeFFT()) 
        : m_fft_rows(fft_rows), m_fft_cols(fft_cols), m_default_fft(default_fft) {}

    value_type fft(const value_type& data, ResizeType resize_type = ResizeType::RESIZE) const;
    value_type ifft(const value_type& data, size_t out_rows = 0, size_t out_cols = 0, ResizeType resize_type = ResizeType::RESIZE) const;

    BlaschkeFFT& get_row_fft(size_t i) { ASSERT(i < m_fft_rows.size(), "Index is out of bounds!"); return m_fft_rows[i]; }
    const BlaschkeFFT& get_row_fft(size_t i) const { return i < m_fft_rows.size() ? m_fft_rows[i] : m_default_fft; }
    BlaschkeFFT& get_col_fft(size_t i) { ASSERT(i < m_fft_cols.size(), "Index is out of bounds!"); return m_fft_cols[i]; }
    const BlaschkeFFT& get_col_fft(size_t i) const { return i < m_fft_cols.size() ? m_fft_cols[i] : m_default_fft; }
    BlaschkeFFT& get_default_fft() { return m_default_fft; }

    void set_fft_rows(const std::vector<BlaschkeFFT>& ffts);
    void set_fft_cols(const std::vector<BlaschkeFFT>& ffts);

    inline size_t rows() const { return m_fft_rows.size(); }
    inline size_t cols() const { return m_fft_cols.size(); }

private:
    std::vector<BlaschkeFFT> m_fft_rows;
    std::vector<BlaschkeFFT> m_fft_cols;
    BlaschkeFFT m_default_fft;

    void fft_linear_sub_matrix(const BlaschkeFFT& bfft, value_type::LinearSubMatrixWrapper sub_matrix, size_t in_size, ResizeType resize_type) const;
    void ifft_linear_sub_matrix(const BlaschkeFFT& bfft, value_type::LinearSubMatrixWrapper sub_matrix, size_t in_size, size_t out_size, ResizeType resize_type) const;
    void fft_rows(value_type& mat, size_t in_size, ResizeType resize_type) const;
    void ifft_rows(value_type& mat, size_t in_size, size_t out_size, ResizeType resize_type) const;
    void fft_cols(value_type& mat, size_t in_size, ResizeType resize_type) const;
    void ifft_cols(value_type& mat, size_t in_size, size_t out_size, ResizeType resize_type) const;
};

};

#endif //FFT2__HPP