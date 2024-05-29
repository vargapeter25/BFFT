#ifndef MPL__HPP
#define MPL__HPP

#include <type_traits>
#include <iterator>
#include <source_location>
#include <iostream>

namespace mpl{

template<typename InputIterator>
concept InputIteratorType = std::input_iterator<InputIterator>;

template<typename T>
concept ContainerType = requires (T container) {
    {container.begin()} -> std::same_as<typename T::iterator>;
    {container.end()} -> std::same_as<typename T::iterator>;
} && std::input_iterator<typename T::iterator>;

template<typename Fun>
concept OptimizerFun = requires(Fun fun) {
    typename Fun::arg_type;
    typename Fun::value_type;
    {fun.argc()} -> std::same_as<size_t>;
    {fun(std::valarray<typename Fun::arg_type>(fun.argc()))} -> std::same_as<typename Fun::value_type>;
};

template<typename T>
concept BooleanTestable = requires(T a) {
    static_cast<bool>(a);
};

template <typename BooleanTestable>
void _assert(const BooleanTestable&& value, std::string_view expr, std::string_view msg, const std::source_location& loc = std::source_location::current()) noexcept
{
   if (!static_cast<bool>(value))
   {
        std::cerr << "Unexpected error: " << msg << '\n';
        std::cerr << "\e[1m" << loc.file_name() << "\e[0m: In function \e[1m`" << loc.function_name() << "`\e[0m:\n";
        std::cerr << "\e[1m" << loc.file_name() << ':'
                  << loc.line() << ':'
                  << loc.column() << "\e[0m: "
                  << expr << " is false" << '\n';
        std::abort();
   }
}

#define ASSERT(x, msg) mpl::_assert((x), #x, msg)

template <typename BooleanTestable>
void _error(const BooleanTestable&& value, std::string_view msg) noexcept
{
   if (!static_cast<bool>(value))
   {
        std::cerr << msg << '\n';
        std::abort();
   }
}

#define ERROR(x, msg) mpl::_error((x), msg)

}

#endif //MPL__HPP