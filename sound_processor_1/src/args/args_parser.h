#pragma once 

#include <string>
#include <vector>

struct FilterDescriptor {
    std::string name;
    std::vector<std::string> params;
};

class ArgsParser {
    public:
    enum class Result {
        Ok, 
        NoArgs, 
        HelpRequested,
        BadArgs
    };

    ArgsParser() = default;

    Result parse(int argc, char* argv[]);

    const std::string& get_input_file() const { return in_file_name_; }
    const std::string& get_output_file() const { return out_file_name_; }
    const std::vector<FilterDescriptor>& get_filters() const { return filter_descriptors_; }

    private:
    std::string in_file_name_;
    std::string out_file_name_;
    std::vector<FilterDescriptor> filter_descriptors_;
};
