#ifndef NELDER_MEAD__HPP
#define NELDER_MEAD__HPP

#include <valarray>
#include <cassert>
#include <cmath>
#include "mpl.hpp"

template<mpl::OptimizerFun Func>
class NelderMead{
public:
    using args_type = std::valarray<typename Func::arg_type>;
    
    struct Point{
        args_type args;
        typename Func::arg_type val;
        bool operator<(const Point& p) const {
            if(val != p.val) return val < p.val;
            for(size_t i = 0; i < args.size(); i++) if(args[i] != p.args[i]) return args[i] < p.args[i];
            return false;
        }
    };
    
    NelderMead(const Func &func, typename Func::arg_type treshold = 0.01, typename Func::arg_type alpha = 1, typename Func::arg_type gamma = 2, typename Func::arg_type rho = 0.5, typename Func::arg_type omega = 0.5) 
        : m_func(func), m_treshold(treshold), m_alpha(alpha), m_gamma(gamma), m_rho(rho), m_omega(omega) {}
    

    args_type find_min(const std::vector<args_type>& starting_points, size_t max_iterations = 50, size_t max_shrink = 3);

    Point create_point(const args_type& args) { return Point{ args, m_func(args) }; }

    static std::vector<args_type> get_starting_points(const args_type& p, typename Func::arg_type shift);
private:
    Func m_func;
    typename Func::arg_type m_treshold;
    typename Func::arg_type m_alpha; // 0 <
    typename Func::arg_type m_gamma; // 1 < 
    typename Func::arg_type m_rho; // 0 < < 0.5
    typename Func::arg_type m_omega; // < 1

    bool check_termination(const std::vector<Point>& points);

    args_type get_centroid(const std::vector<Point>& points);

    typename Func::arg_type distance(const args_type& x, const args_type& y);
};  

template<mpl::OptimizerFun Func>
std::vector<typename NelderMead<Func>::args_type> NelderMead<Func>::get_starting_points(const args_type& p, typename Func::arg_type shift) {
    std::vector<std::valarray<typename Func::arg_type>> starting_points(p.size() + 1);
    starting_points[0] = p;
    for(size_t i = 1; i < starting_points.size(); i++){
        starting_points[i] = p;
        starting_points[i][i-1] += shift;
    }
    return starting_points;
}

template<mpl::OptimizerFun Func>
typename Func::arg_type NelderMead<Func>::distance(const args_type& x, const args_type& y) {
    args_type d = x - y;
    return std::sqrt((d*d).sum());
}

template<mpl::OptimizerFun Func>
bool NelderMead<Func>::check_termination(const std::vector<Point>& points){
    // added to check for function values
    if(std::abs(points.back().val - points.front().val) < 1e-4) return true;
    typename Func::arg_type max_dist = 0;
    for(size_t i = 0; i < points.size(); i++){
        for(size_t j = i + 1; j < points.size(); j++){
            max_dist = std::max(max_dist, distance(points[i].args, points[j].args));
        }
    }
    return max_dist < m_treshold;
}

template<mpl::OptimizerFun Func>
NelderMead<Func>::args_type NelderMead<Func>::get_centroid(const std::vector<Point>& points){
    args_type centroid(0.0, points[0].args.size());
    for(size_t i = 0; i+1 < points.size(); i++){
        centroid += points[i].args;
    }
    ASSERT(points.size() > 1, "At least two points is necessary!");
    centroid /= static_cast<typename Func::arg_type>(points.size() - 1);
    return centroid;
}


template<mpl::OptimizerFun Func>
NelderMead<Func>::args_type NelderMead<Func>::find_min(const std::vector<args_type>& starting_points, size_t max_iterations, size_t max_shrink) {
    ASSERT((starting_points.size() == m_func.argc() + 1), "Number of starting points must be `argc+1`!");

    
    std::vector<Point> points(starting_points.size());
    for(size_t i = 0; i < starting_points.size(); i++){
        points[i] = Point{starting_points[i], m_func(starting_points[i])};
    }
    std::sort(points.begin(), points.end());
    
    // std::cout << "starting points: " << points[0].val << std::endl;

    size_t iterations = 0;
    size_t shrink_counter = 0;
    while(iterations < max_iterations && shrink_counter < max_shrink && !check_termination(points)){
        // counting iterations
        ++iterations;

        args_type centroid = get_centroid(points);

        // case 1: reflection
        args_type reflection_pos = centroid + (centroid - points.back().args) * m_alpha;
        Point reflection = Point{reflection_pos, m_func(reflection_pos)};
        if(points[0].val <= reflection.val && reflection.val < points[points.size() - 2].val) {
            points.back() = reflection;
            std::sort(points.begin(), points.end());
            shrink_counter = 0;
            continue;
        }

        // case 2: expansion
        if(reflection.val < points[0].val){
            args_type expansion_pos = centroid + (reflection_pos - centroid) * m_gamma;
            Point expansion = create_point(expansion_pos);
            if(expansion.val < reflection.val) {
                points.back() = expansion;
            } else{
                points.back() = reflection;
            }
            std::sort(points.begin(), points.end());
            shrink_counter = 0;
            continue;
        }


        // case 3: contraction
        if(reflection.val < points.back().val){
            args_type contraction_pos = centroid + (reflection_pos - centroid) * m_rho;
            Point contraction = create_point(contraction_pos);
            if(contraction.val < reflection.val){
                points.back() = contraction;
                std::sort(points.begin(), points.end());
                shrink_counter = 0;
                continue;
            }
        } else{
            args_type contraction_pos = centroid + (points.back().args - centroid) * m_rho;
            Point contraction = create_point(contraction_pos);
            if(contraction.val < points.back().val){
                points.back() = contraction;
                std::sort(points.begin(), points.end());
                shrink_counter = 0;
                continue;
            }
        }

        // case 4: shrink
        for(size_t i = 1; i < points.size(); i++){
            points[i] = create_point((points[i].args - points[0].args) * m_omega + points[0].args);
        }
        std::sort(points.begin(), points.end());
        shrink_counter++;
    }   

    return points[0].args;
}

#endif //NELDER_MEAD__HPP