#include "args/args_parser.h"
#include <cctype>

static bool is_flag(const char* token) {
    if (token[0] != '-') return false;
    if (token[1] == '\0') return false;          // одинокий '-'
    return std::isalpha(static_cast<unsigned char>(token[1])) != 0;
}

ArgsParser::Result ArgsParser::parse(int argc, char* argv[]) {
    in_file_name_.clear();
    out_file_name_.clear();
    filter_descriptors_.clear();

    if (argc < 2) {
        return Result::NoArgs;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            return Result::HelpRequested;
        }
        else if (arg == "-i") {
            if (i + 1 >= argc || argv[i + 1][0] == '\0' || is_flag(argv[i + 1])) {
                return Result::BadArgs;
            }
            if (!in_file_name_.empty()) {
                return Result::BadArgs;
            }
            in_file_name_ = argv[++i];
        }
        else if (arg == "-o") {
            if (i + 1 >= argc || argv[i + 1][0] == '\0' || is_flag(argv[i + 1])) {
                return Result::BadArgs;
            }
            if (!out_file_name_.empty()) {
                return Result::BadArgs;
            }
            out_file_name_ = argv[++i];
        }
        else if (arg == "-f") {
            if (i + 1 >= argc || is_flag(argv[i + 1])) {
                return Result::BadArgs;
            }
            FilterDescriptor fd;
            fd.name = argv[++i];
            if (fd.name.empty()) {
                return Result::BadArgs;
            }

            while (i + 1 < argc && !is_flag(argv[i + 1])) {
                fd.params.push_back(argv[++i]);
            }
            filter_descriptors_.push_back(fd);
        }
        else {
            return Result::BadArgs;
        }
    }

    return Result::Ok;
}
