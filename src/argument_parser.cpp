#include "../include/argument_parser.hpp"

void Argument::update(const std::vector<std::string>& args, int& pos){
    if(m_is_used){
        pos = -1;
        return;
    }
    for(auto& reader : m_reader_funs){
        auto result = reader->process(args, pos);

        if(pos == -1 || !result.has_value()){
            pos = -1;
            return;
        }

        m_values.push_back(result.value());
    }
    m_is_used = true;
}

template<>
int Argument::read<int>(const std::vector<std::string>& args, int& pos) {
    if(static_cast<int>(args.size()) <= pos) {
        pos = -1;
        return {};
    }
    int result = {};
    try{
        result = std::stoi(args[pos]);
    } catch(...){
        std::cerr << "Cannot convert `" << args[pos] << "` to int." << std::endl;
        pos = -1;
        return {};
    }
    pos++;
    return result;
}

template<>
double Argument::read<double>(const std::vector<std::string>& args, int& pos) {
    if(static_cast<int>(args.size()) <= pos) {
        pos = -1;
        return {};
    }
    double result = {};
    try{
        result = std::stod(args[pos]);
    } catch(...){
        std::cerr << "Cannot convert `" << args[pos] << "` to double." << std::endl;
        pos = -1;
        return {};
    }
    pos++;
    return result;
}

template<>
std::string Argument::read<std::string>(const std::vector<std::string>& args, int& pos) {
    if(static_cast<int>(args.size()) <= pos) {
        pos = -1;
        return {};
    }
    std::string result = std::string(args[pos]);
    pos++;
    return result;
}

Argument& ArgumentParser::add_argument(std::string_view name){
    ASSERT(!name.empty(), "Argument name cannot be empty.");
    ASSERT(!has_argument(name), "Argument names must be unique.");
    if(name[0] == '-'){
        m_flag_args.emplace_back(name);
        return m_flag_args.back();
    } else{
        m_default_args.emplace_back(name);
        return m_default_args.back();
    }
}

Argument& ArgumentParser::get_argument(std::string_view name) { 
    ASSERT(has_argument(name), "Required for argument to exist."); 
    if(has_default(name)){
        return *std::find(m_default_args.begin(), m_default_args.end(), name); 
    } else{
        return *std::find(m_flag_args.begin(), m_flag_args.end(), name); 
    }
}

bool ArgumentParser::parse(int argc, char *argv[]){
    std::vector<std::string> str_args;
    for(int i = 0; i < argc; i++) str_args.push_back(std::string(argv[i]));

    int pos = 0, default_idx = 0;
    while(pos != -1 && pos < static_cast<int>(str_args.size())){
        
        if(str_args[pos].empty()){
            std::cerr << "Empty argument error.";
            return false;
        }

        std::string current_arg = str_args[pos];
        if(has_flag(str_args[pos])){
            Argument& arg = get_argument(str_args[pos]);
            ++pos;
            arg.update(str_args, pos);
        } else if(str_args[pos][0] != '-' && default_idx < static_cast<int>(m_default_args.size())){
            m_default_args[default_idx].update(str_args, pos);
            default_idx++;
        } else{
            pos = -1; 
        }
        if(pos == -1){
            std::cerr << "Invalid or badly specified argument: " << current_arg << std::endl;
            return false;
        }
    }

    if(pos == -1) { 
        std::cerr << "Failed to parse arguments." << std::endl;
        return false;
    }

    for(const Argument& arg : m_flag_args) {
        if(arg.m_is_required && !arg.m_is_used) {
            std::cerr << "Required argument: " << arg.m_name << std::endl;
            return false;
        } else if(arg.m_is_used && arg.m_is_special){
            return true;
        }
    }

    for(const Argument& arg : m_default_args){
        if(arg.m_is_required && !arg.m_is_used){
            std::cerr << "Required argument: " << arg.m_name << std::endl;
            return false;
        }
    }

    return true;
}

std::string ArgumentParser::get_help() const {
    std::string  result = "Command:\n"; 
    result += m_program_name;
    if(!m_flag_args.empty()){
        result += " [";
        for(const Argument& arg : m_flag_args){
            if(result.back() != '['){
                result += '|';
            }
            result += arg.m_name;
        }
        result += ']';
    }
    if(!m_default_args.empty()){
        result += " [";
        for(const Argument& arg : m_default_args){
            if(result.back() != '['){
                result += '|';
            }
            result += arg.m_name;
        }
        result += ']';
    }
    result += '\n';

    if(!m_description.empty()){
        result += "\nDescirption:\n";
        result += m_description + '\n';
    }

    int tab_count = 0;
    for(const Argument& arg : m_default_args){
        tab_count = std::max(tab_count, (static_cast<int>(arg.m_name.size()) + 8) / 8);
    }
    for(const Argument& arg : m_flag_args){
        tab_count = std::max(tab_count, (static_cast<int>(arg.m_name.size()) + 8) / 8);
    }

    result += "\nDefault arguments:\n";
    for(const Argument& arg : m_default_args){
        std::string tabs = std::string(tab_count - (static_cast<int>(arg.m_name.size()) / 8), '\t');
        result += arg.m_name + tabs + arg.get_help();
        if(arg.m_is_required){
            result += " (Required)";
        }
        result += '\n';
    }
    if(m_default_args.empty()){
        result += "No default arguments.\n";
    }
    result += "\nFlags:\n";
    for(const Argument& arg : m_flag_args){
        std::string tabs = std::string(tab_count - (static_cast<int>(arg.m_name.size()) / 8), '\t');
        result += arg.m_name + tabs + arg.get_help();
        if(arg.m_is_required){
            result += " (Required)";
        }
        result += '\n';
    }
    if(m_flag_args.empty()){
        result += "No flags.\n";
    }
    return result;
}