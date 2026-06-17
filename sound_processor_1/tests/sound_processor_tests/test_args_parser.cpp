#include <catch2/catch_test_macros.hpp>

#include "args/args_parser.h"

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

// Проверяет: ArgsParser: no args returns NoArgs
TEST_CASE("ArgsParser: no args returns NoArgs", "[args_parser]") {
    ArgsParser parser;
    std::vector<const char*> argv = {"sound_processor"};
    CHECK(parser.parse(1, const_cast<char**>(argv.data())) == ArgsParser::Result::NoArgs);
}

// Проверяет: ArgsParser: -h returns HelpRequested
TEST_CASE("ArgsParser: -h returns HelpRequested", "[args_parser]") {
    CHECK(parse({"-h"}) == ArgsParser::Result::HelpRequested);
}

// Проверяет: ArgsParser: --help returns HelpRequested
TEST_CASE("ArgsParser: --help returns HelpRequested", "[args_parser]") {
    CHECK(parse({"--help"}) == ArgsParser::Result::HelpRequested);
}

// Проверяет: ArgsParser: valid -i -o returns Ok
TEST_CASE("ArgsParser: valid -i -o returns Ok", "[args_parser]") {
    CHECK(parse({"-i", "input.wav", "-o", "output.wav"}) == ArgsParser::Result::Ok);
}

// Проверяет: ArgsParser: only -i returns Ok
TEST_CASE("ArgsParser: only -i returns Ok", "[args_parser]") {
    CHECK(parse({"-i", "input.wav"}) == ArgsParser::Result::Ok);
}

// Проверяет: ArgsParser: only -o returns Ok
TEST_CASE("ArgsParser: only -o returns Ok", "[args_parser]") {
    CHECK(parse({"-o", "output.wav"}) == ArgsParser::Result::Ok);
}

// Проверяет: ArgsParser: -i missing value returns BadArgs
TEST_CASE("ArgsParser: -i missing value returns BadArgs", "[args_parser]") {
    CHECK(parse({"-i"}) == ArgsParser::Result::BadArgs);
}

// Проверяет: ArgsParser: -o missing value returns BadArgs
TEST_CASE("ArgsParser: -o missing value returns BadArgs", "[args_parser]") {
    CHECK(parse({"-o"}) == ArgsParser::Result::BadArgs);
}

// Проверяет: ArgsParser: -f missing name returns BadArgs
TEST_CASE("ArgsParser: -f missing name returns BadArgs", "[args_parser]") {
    CHECK(parse({"-f"}) == ArgsParser::Result::BadArgs);
}

// Проверяет: ArgsParser: unknown flag returns BadArgs
TEST_CASE("ArgsParser: unknown flag returns BadArgs", "[args_parser]") {
    CHECK(parse({"-z"}) == ArgsParser::Result::BadArgs);
}

// ─── Извлечение данных ───────────────────────────────────────────────────────

// Проверяет: ArgsParser: get_input_file
TEST_CASE("ArgsParser: get_input_file", "[args_parser]") {
    auto p = parse_full({"-i", "in.wav"});
    CHECK(p.get_input_file() == "in.wav");
}

// Проверяет: ArgsParser: get_output_file
TEST_CASE("ArgsParser: get_output_file", "[args_parser]") {
    auto p = parse_full({"-o", "out.wav"});
    CHECK(p.get_output_file() == "out.wav");
}

// Проверяет: ArgsParser: empty input/output when not specified
TEST_CASE("ArgsParser: empty input/output when not specified", "[args_parser]") {
    auto p = parse_full({"-f", "ampl", "0.8"});
    CHECK(p.get_input_file().empty());
    CHECK(p.get_output_file().empty());
}

// Проверяет: ArgsParser: single filter with params
TEST_CASE("ArgsParser: single filter with params", "[args_parser]") {
    auto p = parse_full({"-f", "ampl", "0.8"});
    const auto& filters = p.get_filters();
    REQUIRE(filters.size() == 1);
    CHECK(filters[0].name == "ampl");
    REQUIRE(filters[0].params.size() == 1);
    CHECK(filters[0].params[0] == "0.8");
}

// Проверяет: ArgsParser: multiple filters
TEST_CASE("ArgsParser: multiple filters", "[args_parser]") {
    auto p = parse_full({"-f", "ampl", "0.8", "-f", "timestretch", "2"});
    const auto& filters = p.get_filters();
    REQUIRE(filters.size() == 2);
    CHECK(filters[0].name == "ampl");
    CHECK(filters[1].name == "timestretch");
    CHECK(filters[1].params[0] == "2");
}

// Проверяет: ArgsParser: filter with no params
TEST_CASE("ArgsParser: filter with no params", "[args_parser]") {
    auto p = parse_full({"-f", "normalize"});
    const auto& filters = p.get_filters();
    REQUIRE(filters.size() == 1);
    CHECK(filters[0].name == "normalize");
    CHECK(filters[0].params.empty());
}

// Проверяет: ArgsParser: negative number is a param not a flag
TEST_CASE("ArgsParser: negative number is a param not a flag", "[args_parser]") {
    // -0.5 не должен восприниматься как флаг
    auto p = parse_full({"-f", "ampl", "-0.5"});
    const auto& filters = p.get_filters();
    REQUIRE(filters.size() == 1);
    CHECK(filters[0].params[0] == "-0.5");
}

// Проверяет: ArgsParser: silence with 3 params
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

// Проверяет: ArgsParser: generator sin params
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

// Проверяет: ArgsParser: full example from TZ
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

// Проверяет: ArgsParser: duplicate -i returns BadArgs
TEST_CASE("ArgsParser: duplicate -i returns BadArgs", "[args_parser]") {
    CHECK(parse({"-i", "a.wav", "-i", "b.wav"}) == ArgsParser::Result::BadArgs);
}

// Проверяет: ArgsParser: duplicate -o returns BadArgs
TEST_CASE("ArgsParser: duplicate -o returns BadArgs", "[args_parser]") {
    CHECK(parse({"-o", "a.wav", "-o", "b.wav"}) == ArgsParser::Result::BadArgs);
}

// Проверяет: ArgsParser: -f followed by flag returns BadArgs
TEST_CASE("ArgsParser: -f followed by flag returns BadArgs", "[args_parser]") {
    CHECK(parse({"-f", "-i"}) == ArgsParser::Result::BadArgs);
}

// Проверяет: ArgsParser: empty input/output values return BadArgs
TEST_CASE("ArgsParser: empty input/output values return BadArgs", "[args_parser]") {
    CHECK(parse({"-i", ""}) == ArgsParser::Result::BadArgs);
    CHECK(parse({"-o", ""}) == ArgsParser::Result::BadArgs);
}

// Проверяет: ArgsParser: parse clears previous state
TEST_CASE("ArgsParser: parse clears previous state", "[args_parser]") {
    ArgsParser parser;
    std::vector<const char*> argv = {"sound_processor", "-i", "first.wav"};
    CHECK(parser.parse(static_cast<int>(argv.size()), const_cast<char**>(argv.data())) == ArgsParser::Result::Ok);

    argv = {"sound_processor", "-o", "second.wav"};
    CHECK(parser.parse(static_cast<int>(argv.size()), const_cast<char**>(argv.data())) == ArgsParser::Result::Ok);
    CHECK(parser.get_input_file().empty());
    CHECK(parser.get_output_file() == "second.wav");
    CHECK(parser.get_filters().empty());
}

// Проверяет: ArgsParser: filter params preserve trailing text for producer validation
TEST_CASE("ArgsParser: filter params preserve trailing text for producer validation", "[args_parser]") {
    auto p = parse_full({"-f", "ampl", "1abc"});
    REQUIRE(p.get_filters().size() == 1);
    CHECK(p.get_filters()[0].params[0] == "1abc");
}
