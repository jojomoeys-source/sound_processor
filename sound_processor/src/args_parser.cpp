#include "args_parser.h"
#include <cctype>

// Токен является флагом (например, -i, -o, -f, -h), если:
//   - начинается с '-'
//   - второй символ — буква (не цифра и не точка)
// Токены вида "-0.5", "-1", "-.5" — это отрицательные числа, не флаги.
static bool is_flag(const char* token) {
    if (token[0] != '-') return false;
    if (token[1] == '\0') return false;          // одинокий '-'
    return std::isalpha(static_cast<unsigned char>(token[1])) != 0;
}

ArgsParser::Result ArgsParser::parse(int argc, char* argv[]) {

    if (argc < 2) {
        return Result::NoArgs;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            return Result::HelpRequested;
        }
        else if (arg == "-i") {
            if (i + 1 < argc) {
                in_file_name_ = argv[++i];
            } else {
                return Result::BadArgs;
            }
        }
        else if (arg == "-o") {
            if (i + 1 < argc) {
                out_file_name_ = argv[++i];
            } else {
                return Result::BadArgs;
            }
        }
        else if (arg == "-f") {
            if (i + 1 < argc) {
                FilterDescriptor fd;
                fd.name = argv[++i];

                // Собираем параметры фильтра до тех пор, пока следующий токен
                // не является флагом (-i, -o, -f, ...).
                // Отрицательные числа (-0.5, -1) — не флаги, берём их как параметры.
                while (i + 1 < argc && !is_flag(argv[i + 1])) {
                    fd.params.push_back(argv[++i]);
                }
                filter_descriptors_.push_back(fd);
            } else {
                return Result::BadArgs;
            }
        }
        else {
            return Result::BadArgs;
        }
    }

    return Result::Ok;
}
