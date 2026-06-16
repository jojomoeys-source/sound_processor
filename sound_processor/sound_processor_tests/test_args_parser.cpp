#include <catch2/catch_test_macros.hpp>

#include "args_parser.h"

// Вспомогательная функция для построения argv из вектора строк
// (argv[0] — имя программы всегда присутствует)
static ArgsParser::Result parse(std::vector<const char*> args) {
    // argv[0] = program name
    args.insert(args.begin(), "sound_processor");
    ArgsParser parser;
    return parser.parse(static_cast<int>(args.size()),
                        const_cast<char**>(args.data()));
}

static ArgsParser parse_full(std::vector<const char*> args) {
    args.insert(args.begin(), "sound_processor");
    ArgsParser parser;
    parser.parse(static_cast<int>(args.size()),
                 const_cast<char**>(args.data()));
    return parser;
}

// ─── Результаты парсинга ─────────────────────────────────────────────────────

TEST_CASE("ArgsParser: no args returns NoArgs", "[args_parser]") {
    ArgsParser parser;
    std::vector<const char*> argv = {"sound_processor"};
    CHECK(parser.parse(1, const_cast<char**>(argv.data())) == ArgsParser::Result::NoArgs);
}

TEST_CASE("ArgsParser: -h returns HelpRequested", "[args_parser]") {
    CHECK(parse({"-h"}) == ArgsParser::Result::HelpRequested);
}

TEST_CASE("ArgsParser: --help returns HelpRequested", "[args_parser]") {
    CHECK(parse({"--help"}) == ArgsParser::Result::HelpRequested);
}

TEST_CASE("ArgsParser: valid -i -o returns Ok", "[args_parser]") {
    CHECK(parse({"-i", "input.wav", "-o", "output.wav"}) == ArgsParser::Result::Ok);
}

TEST_CASE("ArgsParser: only -i returns Ok", "[args_parser]") {
    CHECK(parse({"-i", "input.wav"}) == ArgsParser::Result::Ok);
}

TEST_CASE("ArgsParser: only -o returns Ok", "[args_parser]") {
    CHECK(parse({"-o", "output.wav"}) == ArgsParser::Result::Ok);
}

TEST_CASE("ArgsParser: -i missing value returns BadArgs", "[args_parser]") {
    CHECK(parse({"-i"}) == ArgsParser::Result::BadArgs);
}

TEST_CASE("ArgsParser: -o missing value returns BadArgs", "[args_parser]") {
    CHECK(parse({"-o"}) == ArgsParser::Result::BadArgs);
}

TEST_CASE("ArgsParser: -f missing name returns BadArgs", "[args_parser]") {
    CHECK(parse({"-f"}) == ArgsParser::Result::BadArgs);
}

TEST_CASE("ArgsParser: unknown flag returns BadArgs", "[args_parser]") {
    CHECK(parse({"-z"}) == ArgsParser::Result::BadArgs);
}

// ─── Извлечение данных ───────────────────────────────────────────────────────

TEST_CASE("ArgsParser: get_input_file", "[args_parser]") {
    auto p = parse_full({"-i", "in.wav"});
    CHECK(p.get_input_file() == "in.wav");
}

TEST_CASE("ArgsParser: get_output_file", "[args_parser]") {
    auto p = parse_full({"-o", "out.wav"});
    CHECK(p.get_output_file() == "out.wav");
}

TEST_CASE("ArgsParser: empty input/output when not specified", "[args_parser]") {
    auto p = parse_full({"-f", "ampl", "0.8"});
    CHECK(p.get_input_file().empty());
    CHECK(p.get_output_file().empty());
}

TEST_CASE("ArgsParser: single filter with params", "[args_parser]") {
    auto p = parse_full({"-f", "ampl", "0.8"});
    const auto& filters = p.get_filters();
    REQUIRE(filters.size() == 1);
    CHECK(filters[0].name == "ampl");
    REQUIRE(filters[0].params.size() == 1);
    CHECK(filters[0].params[0] == "0.8");
}

TEST_CASE("ArgsParser: multiple filters", "[args_parser]") {
    auto p = parse_full({"-f", "ampl", "0.8", "-f", "timestretch", "2"});
    const auto& filters = p.get_filters();
    REQUIRE(filters.size() == 2);
    CHECK(filters[0].name == "ampl");
    CHECK(filters[1].name == "timestretch");
    CHECK(filters[1].params[0] == "2");
}

TEST_CASE("ArgsParser: filter with no params", "[args_parser]") {
    auto p = parse_full({"-f", "normalize"});
    const auto& filters = p.get_filters();
    REQUIRE(filters.size() == 1);
    CHECK(filters[0].name == "normalize");
    CHECK(filters[0].params.empty());
}

TEST_CASE("ArgsParser: negative number is a param not a flag", "[args_parser]") {
    // -0.5 не должен восприниматься как флаг
    auto p = parse_full({"-f", "ampl", "-0.5"});
    const auto& filters = p.get_filters();
    REQUIRE(filters.size() == 1);
    CHECK(filters[0].params[0] == "-0.5");
}

TEST_CASE("ArgsParser: silence with 3 params", "[args_parser]") {
    auto p = parse_full({"-f", "silence", "sec", "0.2", "0.4"});
    const auto& filters = p.get_filters();
    REQUIRE(filters.size() == 1);
    CHECK(filters[0].name == "silence");
    REQUIRE(filters[0].params.size() == 3);
    CHECK(filters[0].params[0] == "sec");
    CHECK(filters[0].params[1] == "0.2");
    CHECK(filters[0].params[2] == "0.4");
}

TEST_CASE("ArgsParser: generator sin params", "[args_parser]") {
    auto p = parse_full({"-f", "generator", "sin", "440", "2000"});
    const auto& filters = p.get_filters();
    REQUIRE(filters.size() == 1);
    CHECK(filters[0].name == "generator");
    REQUIRE(filters[0].params.size() == 3);
    CHECK(filters[0].params[0] == "sin");
    CHECK(filters[0].params[1] == "440");
    CHECK(filters[0].params[2] == "2000");
}

TEST_CASE("ArgsParser: full example from TZ", "[args_parser]") {
    // sound_processor -i input.wav -o output.wav -f ampl 0.8 -f timestretch 2
    auto p = parse_full({"-i", "input.wav", "-o", "output.wav",
                         "-f", "ampl", "0.8",
                         "-f", "timestretch", "2"});
    CHECK(p.get_input_file()  == "input.wav");
    CHECK(p.get_output_file() == "output.wav");
    const auto& f = p.get_filters();
    REQUIRE(f.size() == 2);
    CHECK(f[0].name == "ampl");
    CHECK(f[0].params[0] == "0.8");
    CHECK(f[1].name == "timestretch");
    CHECK(f[1].params[0] == "2");
}