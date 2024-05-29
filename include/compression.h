#ifndef COMPRESSION__H
#define COMPRESSION__H

#include <vector>
#include <cassert>
#include "mpl.hpp"
#include "fft.hpp"
#include "fft2.hpp"
#include "matrix.hpp"
#include "utils.hpp"
#include "interpolation.hpp"
#include "utils.hpp"

namespace bfft{

struct CompressedData1D{
    struct Coefficent{
        size_t id;
        BlaschkeFunction::value_type value;
    };
    std::vector<Coefficent> compr_data;
    std::vector<BlaschkeFunction::value_type> args;
    size_t transformed_size;
    size_t original_size;
    BlaschkeFFT::ResizeType resize_type;
};

class Compressor1D{
public:
    Compressor1D(double ratio, BlaschkeFFT::ResizeType resize_type = BlaschkeFFT::ResizeType::RESIZE) 
                 : m_bfft(), m_ratio(ratio), m_resize_type(resize_type) {}

    Compressor1D(const BlaschkeFFT& bfft, double ratio, 
                 BlaschkeFFT::ResizeType resize_type = BlaschkeFFT::ResizeType::RESIZE) 
                 : m_bfft(bfft), m_ratio(ratio), m_resize_type(resize_type) { ASSERT((0.0 < ratio && ratio <= 1.0), "Ratio is not between boundaries (0, 1]!"); }

    Compressor1D(const std::vector<BlaschkeFunction::value_type> &params, double ratio, 
                 BlaschkeFFT::ResizeType resize_type = BlaschkeFFT::ResizeType::RESIZE) 
                 : m_bfft(FunctionSystem(params)), m_ratio(ratio), m_resize_type(resize_type) {}
    
    CompressedData1D compress(const std::vector<BlaschkeFunction::value_type>& source) const;

    std::vector<BlaschkeFunction::value_type> decompress(const CompressedData1D& data) const;

    std::vector<BlaschkeFunction::value_type> this_decompress(const CompressedData1D& data) const;

    double compression_error(const std::vector<BlaschkeFunction::value_type>& data) const;
private:
    BlaschkeFFT m_bfft;
    double m_ratio;
    BlaschkeFFT::ResizeType m_resize_type;
};

};

#endif //COMPRESSION__H