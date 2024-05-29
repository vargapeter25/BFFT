#ifndef MATRIX__HPP
#define MATRIX__HPP

#include <algorithm>
#include <iterator>
#include <cstdio>
#include <vector>
#include <compare>

#include "utils.hpp"
#include "mpl.hpp"

namespace bfft::matrix{
    
template<typename T>
struct MatrixConstIterator;

template<typename T>
struct MatrixIterator{
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = typename std::vector<T>::iterator::difference_type;
    using value_type        = T;
    using pointer           = value_type*;
    using reference         = value_type&;

    MatrixIterator(typename std::vector<T>::iterator iter, difference_type stride) : m_iter(iter), m_stride(stride) {}

    reference operator*() const { return *m_iter; }
    pointer operator->() const { return m_iter; }
    reference operator[](difference_type n) const { return *((*this) + n); }

    MatrixIterator& operator++() { m_iter += m_stride; return *this; }
    MatrixIterator& operator--() { m_iter -= m_stride; return *this; }
    
    MatrixIterator operator++(int) { MatrixIterator tmp = *this; ++(*this); return tmp; }
    MatrixIterator operator--(int) { MatrixIterator tmp = *this; --(*this); return tmp; }

    MatrixIterator operator+(difference_type n) const { return MatrixIterator(m_iter + m_stride * n, m_stride); }
    MatrixIterator operator-(difference_type n) const { return MatrixIterator(m_iter - m_stride * n, m_stride); }

    MatrixIterator& operator+=(difference_type n) { m_iter += m_stride * n; return *this; }
    MatrixIterator& operator-=(difference_type n) { m_iter -= m_stride * n; return *this; }

    friend MatrixIterator operator+(difference_type n, const MatrixIterator& i) { return i + n; }
    friend difference_type operator-(const MatrixIterator& a, const MatrixIterator& b) { return (a.m_iter - b.m_iter) / a.m_stride; }

    friend bool operator==(const MatrixIterator &a, const MatrixIterator& b) { return a.m_iter == b.m_iter; }
    friend bool operator!=(const MatrixIterator &a, const MatrixIterator& b) { return a.m_iter != b.m_iter; }
    friend constexpr std::weak_ordering operator<=>(const MatrixIterator &a, const MatrixIterator &b) { return a.m_iter <=> b.m_iter; }

    friend struct MatrixConstIterator<T>;

private:
    typename std::vector<T>::iterator m_iter;
    difference_type m_stride;
};

template<typename T>
struct MatrixConstIterator{
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = typename std::vector<T>::const_iterator::difference_type;
    using value_type        = T;
    using pointer           = const value_type*;
    using reference         = const value_type&;

    MatrixConstIterator(typename std::vector<T>::const_iterator iter, difference_type stride) : m_iter(iter), m_stride(stride) {}
    MatrixConstIterator(MatrixIterator<T> iterator) : m_iter(iterator.m_iter), m_stride(iterator.m_stride) {}

    reference operator*() const { return *m_iter; }
    pointer operator->() const { return m_iter; }
    reference operator[](difference_type n) const { return *((*this) + n); }

    MatrixConstIterator& operator++() { m_iter += m_stride; return *this; }
    MatrixConstIterator& operator--() { m_iter -= m_stride; return *this; }
    
    MatrixConstIterator operator++(int) { MatrixConstIterator tmp = *this; ++(*this); return tmp; }
    MatrixConstIterator operator--(int) { MatrixConstIterator tmp = *this; --(*this); return tmp; }

    MatrixConstIterator operator+(difference_type n) const { return MatrixConstIterator(m_iter + m_stride * n, m_stride); }
    MatrixConstIterator operator-(difference_type n) const { return MatrixConstIterator(m_iter - m_stride * n, m_stride); }

    MatrixConstIterator& operator+=(difference_type n) { m_iter += m_stride * n; return *this; }
    MatrixConstIterator& operator-=(difference_type n) { m_iter -= m_stride * n; return *this; }

    friend MatrixConstIterator operator+(difference_type n, const MatrixConstIterator& i) { return i + n; }
    friend difference_type operator-(const MatrixConstIterator& a, const MatrixConstIterator& b) { return (a.m_iter - b.m_iter) / a.m_stride; }

    friend bool operator==(const MatrixConstIterator &a, const MatrixConstIterator &b) { return a.m_iter == b.m_iter; }
    friend bool operator!=(const MatrixConstIterator &a, const MatrixConstIterator &b) { return a.m_iter != b.m_iter; }
    friend std::weak_ordering operator<=>(const MatrixConstIterator& a, const MatrixConstIterator& b) { return b.m_iter <=> a.m_iter; }

private:
    typename std::vector<T>::const_iterator m_iter;
    difference_type m_stride;
};

template<typename T>
struct Matrix{
public:
    using value_type = T;

    struct ConstLinearSubMatrixWrapper;

    struct LinearSubMatrixWrapper{
        using iterator = MatrixIterator<T>;
        using const_iterator = MatrixConstIterator<T>;

        LinearSubMatrixWrapper(std::vector<T>::iterator begin, std::vector<T>::iterator end, std::vector<T>::iterator::difference_type stride) : m_begin(begin, stride), m_end(end, stride) {}

        inline iterator::reference operator[](iterator::difference_type n) { return begin()[n]; }
        inline const_iterator::reference operator[](const_iterator::difference_type n) const { return cbegin()[n]; } 

        inline iterator begin() { return m_begin; }
        inline const_iterator begin() const { return m_begin; }
        inline const_iterator cbegin() const { return const_iterator(m_begin); }
        inline iterator end() { return m_end; }
        inline const_iterator end() const { return m_end; }
        inline const_iterator cend() const { return const_iterator(m_end); }

        friend struct ConstLinearSubMatrixWrapper;
    private:
        iterator m_begin;
        iterator m_end;
    };

    struct ConstLinearSubMatrixWrapper{
        using iterator = MatrixConstIterator<T>;

        ConstLinearSubMatrixWrapper(std::vector<T>::const_iterator begin, std::vector<T>::const_iterator end, std::vector<T>::const_iterator::difference_type stride) : m_begin(begin, stride), m_end(end, stride) {}
        ConstLinearSubMatrixWrapper(LinearSubMatrixWrapper wrapper) : m_begin(wrapper.m_begin), m_end(wrapper.m_end) {}

        inline iterator::reference operator[](iterator::difference_type n) const { return m_begin[n]; } 

        inline iterator begin() const { return m_begin; }
        inline iterator end() const { return m_end; }
    private:
        iterator m_begin;
        iterator m_end;
    };

    Matrix(size_t rows, size_t cols) : m_rows(rows), m_cols(cols), m_data(rows * cols) {}
    template<mpl::InputIteratorType InputIterator>
    Matrix(size_t rows, size_t cols, InputIterator first, InputIterator last) : m_rows(rows), m_cols(cols), m_data(rows * cols) { std::copy(first, last, m_data.begin()); }
    template<mpl::ContainerType Container>
    Matrix(size_t rows, size_t cols, const Container& container) : Matrix(rows, cols, container.begin(), container.end()) {}

    void transpose() { m_transpose = !m_transpose; }
    void mem_transpose();

    inline LinearSubMatrixWrapper operator[](size_t i) { return get_row(i); }
    inline const ConstLinearSubMatrixWrapper operator[](size_t i) const { return get_row(i); }


    inline LinearSubMatrixWrapper get_row(size_t i) { return m_transpose ? _col(i) : _row(i); }
    inline ConstLinearSubMatrixWrapper get_row(size_t i) const { return m_transpose ? _colc(i) : _rowc(i); }
    inline LinearSubMatrixWrapper get_col(size_t i) { return m_transpose ? _row(i) : _col(i); }
    inline ConstLinearSubMatrixWrapper get_col(size_t i) const { return m_transpose ? _rowc(i) : _colc(i); }

    std::vector<LinearSubMatrixWrapper> get_rows();
    std::vector<ConstLinearSubMatrixWrapper> get_rows() const;
    std::vector<LinearSubMatrixWrapper> get_cols();
    std::vector<ConstLinearSubMatrixWrapper> get_cols() const;

    inline size_t rows() const { return m_transpose ? m_cols : m_rows; }
    inline size_t cols() const { return m_transpose ? m_rows : m_cols; }

    inline const std::vector<T>& data() const { return m_data; }
    inline std::vector<T>& data() { return m_data; }

    template<mpl::InputIteratorType InputIterator>
    static void copy_to(LinearSubMatrixWrapper _data, InputIterator first, InputIterator last);
    template<mpl::InputIteratorType InputIterator>
    static void copy_to_zeros(LinearSubMatrixWrapper _data, InputIterator first, InputIterator last);
    template<mpl::InputIteratorType InputIterator>
    static void copy_to_continous(LinearSubMatrixWrapper _data, InputIterator first, InputIterator last);
    template<mpl::InputIteratorType InputIterator>
    static void copy_to_repeat(LinearSubMatrixWrapper _data, InputIterator first, InputIterator last);

    static void copy_to(Matrix& _data, const Matrix& source);
    static void copy_to_pos(Matrix& _data, const Matrix& source, size_t offset_row, size_t offset_col);
    static Matrix submatrix_from_pos(const Matrix& data, size_t offset_row, size_t offset_col, size_t rows, size_t cols);
private:
    size_t m_rows, m_cols;
    std::vector<T> m_data;
    bool m_transpose = false;

    inline LinearSubMatrixWrapper _row(size_t i) { return LinearSubMatrixWrapper(m_data.begin() + m_cols * i, m_data.begin() + m_cols * (i+1), 1); }
    inline ConstLinearSubMatrixWrapper _rowc(size_t i) const { return ConstLinearSubMatrixWrapper(m_data.cbegin() + m_cols * i, m_data.cbegin() + m_cols * (i+1), 1); }
    inline LinearSubMatrixWrapper _col(size_t i) { return LinearSubMatrixWrapper(m_data.begin() + i, m_data.end() + i, m_cols); }
    inline ConstLinearSubMatrixWrapper _colc(size_t i) const { return ConstLinearSubMatrixWrapper(m_data.cbegin() + i, m_data.cend() + i, m_cols); }
};

template<typename T>
void Matrix<T>::mem_transpose() {
    if(m_transpose){
        m_transpose = false;
        return;
    }
    std::vector<T> new_data(m_cols * m_rows);
    for(size_t i = 0; i < m_rows; i++){
        for(size_t j = 0; j < m_cols; j++){
            new_data[m_rows * j + i] = m_data[m_cols * i + j];
        }
    }
    std::swap(m_data, new_data);
    std::swap(m_rows, m_cols);
}

template<typename T>
std::vector<typename Matrix<T>::LinearSubMatrixWrapper> Matrix<T>::get_rows() {
    std::vector<LinearSubMatrixWrapper> rows_wrapper;
    rows_wrapper.reserve(rows());
    for(size_t i = 0; i < rows(); i++){
        rows_wrapper.push_back(get_row(i));
    }
    return rows_wrapper;
}

template<typename T>
std::vector<typename Matrix<T>::ConstLinearSubMatrixWrapper> Matrix<T>::get_rows() const {
    std::vector<ConstLinearSubMatrixWrapper> rows_wrapper;
    rows_wrapper.reserve(rows());
    for(size_t i = 0; i < rows(); i++){
        rows_wrapper.push_back(get_row(i));
    }
    return rows_wrapper;
}

template<typename T>
std::vector<typename Matrix<T>::LinearSubMatrixWrapper> Matrix<T>::get_cols() {
    std::vector<LinearSubMatrixWrapper> cols_wrapper;
    cols_wrapper.reserve(cols());
    for(size_t i = 0; i < cols(); i++){
        cols_wrapper.push_back(get_col(i));
    }
    return cols_wrapper;
}

template<typename T>
std::vector<typename Matrix<T>::ConstLinearSubMatrixWrapper> Matrix<T>::get_cols() const {
    std::vector<ConstLinearSubMatrixWrapper> cols_wrapper;
    cols_wrapper.reserve(cols());
    for(size_t i = 0; i < cols(); i++){
        cols_wrapper.push_back(get_col(i));
    }
    return cols_wrapper;
}

template<typename T>
template<mpl::InputIteratorType InputIterator>
void Matrix<T>::copy_to(LinearSubMatrixWrapper _data, InputIterator first, InputIterator last){
    size_t data_length = std::distance(first, last);
    size_t container_length = std::distance(_data.begin(), _data.end());
    if(data_length <= container_length){
        std::copy(first, last, _data.begin());
    } else {
        std::copy(first, first + container_length, _data.begin());
    }
}

template<typename T>
template<mpl::InputIteratorType InputIterator>
void Matrix<T>::copy_to_zeros(LinearSubMatrixWrapper _data, InputIterator first, InputIterator last){
    copy_to(_data, first, last);
    size_t data_length = std::distance(first, last);
    size_t container_length = std::distance(_data.begin(), _data.end());
    if(data_length < container_length){
        std::fill(_data.begin() + data_length, _data.end(), 0);
    }
}

template<typename T>
template<mpl::InputIteratorType InputIterator>
void Matrix<T>::copy_to_continous(LinearSubMatrixWrapper _data, InputIterator first, InputIterator last){
    copy_to(_data, first, last);
    size_t data_length = std::distance(first, last);
    size_t container_length = std::distance(_data.begin(), _data.end());
    if(data_length < container_length){
        std::fill(_data.begin() + data_length, _data.end(), *std::prev(last));
    }
}

template<typename T>
template<mpl::InputIteratorType InputIterator>
void Matrix<T>::copy_to_repeat(LinearSubMatrixWrapper _data, InputIterator first, InputIterator last){
    size_t data_length = std::distance(first, last);
    typename LinearSubMatrixWrapper::iterator it = _data.begin();
    while(it < _data.end()){
        size_t remaining_space = _data.end() - it;
        if(remaining_space < data_length){
            std::copy(first, first + remaining_space, it);
        } else {
            std::copy(first, last, it);
        }
        it += data_length;
    }
}

template<typename T>
void Matrix<T>::copy_to(Matrix& _data, const Matrix& source){
    for(size_t i = 0; i < std::min(_data.rows(), source.rows()); i++){
        auto source_row = source.get_row(i);
        _data.copy_to(_data.get_row(i), source_row.begin(), source_row.end());
    }
}

template<typename T>
void Matrix<T>::copy_to_pos(Matrix& _data, const Matrix& source, size_t offset_row, size_t offset_col){
    for(size_t i = 0; i < source.rows() && i + offset_row < _data.rows(); i++){
        for(size_t j = 0; j < source.cols() && j + offset_col < _data.cols(); j++){
            _data[offset_row + i][offset_col + j] = source[i][j];
        }
    }
}


template<typename T>
Matrix<T> Matrix<T>::submatrix_from_pos(const Matrix& data, size_t offset_row, size_t offset_col, size_t rows, size_t cols){
    Matrix result(rows, cols);
     for(size_t i = 0; i < rows && i + offset_row < data.rows(); i++){
        for(size_t j = 0; j < cols && j + offset_col < data.cols(); j++){
            result[i][j] = data[offset_row + i][offset_col + j];
        }
    }
    return result;
}

}

#endif //MATRIX__HPP
