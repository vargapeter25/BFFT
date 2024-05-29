#ifndef ARGUMENT_PARSER__HPP
#define ARGUMENT_PARSER__HPP

#include <string>
#include <iostream>
#include <iomanip>
#include <variant>
#include <vector>
#include <functional>
#include <typeinfo>
#include <unordered_map>
#include <optional>
#include <any>
#include <memory>
#include "mpl.hpp"

class ArgReaderBase {
public:
    virtual std::optional<std::any> process(const std::vector<std::string>&, int&) const = 0;
    virtual ~ArgReaderBase() {}
};

template<typename T>
class ArgReader : public ArgReaderBase{
public:
    ArgReader(std::function<T(const std::vector<std::string>&, int&)> reader,
              std::function<bool(T)> checker = [](const T&) { return true; }) :
                m_reader(reader), m_checker(checker) {}
    std::optional<std::any> process(const std::vector<std::string>& args, int& pos) const override;
private:
    std::function<T(const std::vector<std::string>&, int&)> m_reader;
    std::function<bool(T)> m_checker;
};

template<typename T>
std::optional<std::any> ArgReader<T>::process(const std::vector<std::string>& args, int& pos) const {
    T result = m_reader(args, pos);
    if(pos == -1 || !m_checker(result)) return {};
    return std::make_any<T>(result);
}

class ArgumentParser;

class Argument{
public:
    friend ArgumentParser;

    Argument(std::string_view _name){
        m_name = _name;
    }

    const std::vector<std::any>& get() const { return m_values; }

    bool operator==(const Argument &arg) const { return m_name == arg.m_name; }
    bool operator==(std::string_view _name) const { return m_name == _name; }

    bool has_params() const { return !m_reader_funs.empty(); }
    std::string get_help() const { return m_help_msg; }

    template<typename T>
    static T read(const std::vector<std::string>& args, int& pos);

    template<typename T>
    Argument& add(std::function<bool(T)> checker = [](std::any) -> bool { return true; }, 
                  std::function<T(const std::vector<std::string>&, int&)> reader = read<T>) {             
        m_reader_funs.emplace_back(std::make_unique<ArgReader<T>>(reader, checker)); return *this; 
    }
    Argument& help(std::string_view msg) { m_help_msg = msg; return *this; }
    Argument& required() { m_is_required = true; return *this; }
    Argument& special() { m_is_special = true; return *this; }

    void update(const std::vector<std::string>& args, int& pos);

private:
    std::string m_name = "";
    std::string m_help_msg = "";
    std::vector<std::any> m_values;
    std::vector<std::unique_ptr<ArgReaderBase>> m_reader_funs;
    bool m_is_used = false;
    bool m_is_required = false;
    bool m_is_special = false;
};

template<typename T>
T Argument::read(const std::vector<std::string>&, int& pos) { 
    pos = -1; 
    std::cerr << "Invalid argument." << std::endl; 
    return {}; 
}

template<>
int Argument::read<int>(const std::vector<std::string>& args, int& pos);
template<>
double Argument::read<double>(const std::vector<std::string>& args, int& pos);
template<>
std::string Argument::read<std::string>(const std::vector<std::string>& args, int& pos);

class ArgumentParser{
public:

    ArgumentParser(std::string_view name, std::string_view descirption = "") 
        : m_program_name(name), m_description(descirption) {}

    Argument& add_argument(std::string_view name);

    bool has_argument(std::string_view name) const { 
        return has_flag(name) || has_default(name);
    }
   
    bool used_argument(std::string_view name) { 
        ASSERT(has_argument(name), "Only added arguments can be queried.");
        return get_argument(name).m_is_used; 
    }

    Argument& get_argument(std::string_view name);

    template<typename T>
    T get_value(std::string_view name, size_t idx = 0);

    bool parse(int argc, char *argv[]);
    
    std::string get_help() const;

private:
    std::vector<Argument> m_flag_args;
    std::vector<Argument> m_default_args;
    std::string m_program_name;
    std::string m_description;

    bool has_flag(std::string_view name) const {
        return std::find(m_flag_args.begin(), m_flag_args.end(), name) != m_flag_args.end();
    }
    bool has_default(std::string_view name) const {
        return std::find(m_default_args.begin(), m_default_args.end(), name) != m_default_args.end();
    }

    std::vector<std::any> get_values(std::string_view name) { return get_argument(name).get(); }
};

template<typename T>
T ArgumentParser::get_value(std::string_view name, size_t idx) { 
    ASSERT(used_argument(name), "Required data"); 
    return std::any_cast<T>(get_values(name)[idx]); 
}

#endif //ARGUMENT_PARSER__HPP