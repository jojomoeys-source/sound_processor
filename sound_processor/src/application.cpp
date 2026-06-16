#include "application.h"

#include "filter_producers.h"
#include "wav_io.h"
#include "waveform.h"

#include <iostream>

void Application::configure() {
    converter_.add_filter_producer("mute", filter_producers::make_mute);
    converter_.add_filter_producer("gain", filter_producers::make_gain);
    converter_.add_filter_producer("mix",  filter_producers::make_mix);
}

int Application::start(int argc, char* argv[]) {
    // 1. Парсинг аргументов командной строки
    ArgsParser parser;
    const ArgsParser::Result parse_result = parser.parse(argc, argv);

    if (parse_result == ArgsParser::Result::NoArgs ||
        parse_result == ArgsParser::Result::HelpRequested) {
        print_help();
        return 0;
    }

    if (parse_result == ArgsParser::Result::BadArgs) {
        std::cerr << "Error: invalid arguments. Run without arguments for usage.\n";
        return 1;
    }

    // 2. Построение пайплайна из распарсенных дескрипторов
    Pipeline pipeline = converter_.create_pipeline(parser.get_filters());

    // 3. Загрузка входного сигнала (опционально)
    Waveform waveform;
    const std::string& in_file = parser.get_input_file();
    if (!in_file.empty()) {
        std::cout << "Reading: " << in_file << "\n";
        waveform = WavReader::read(in_file);
    }

    // 4. Применение пайплайна
    pipeline.apply(waveform);

    // 5. Запись результата (опционально)
    const std::string& out_file = parser.get_output_file();
    if (!out_file.empty()) {
        std::cout << "Writing: " << out_file << "\n";
        WavWriter::write(out_file, waveform);
    }

    std::cout << "Done.\n";
    return 0;
}

void Application::print_help() const {
    std::cout
        << "Usage: sound_processor [-i <input.wav>] -o <output.wav>"
           " [-f <filter> [args...]] ...\n"
        << "\n"
        << "Filters:\n"
        << "  mute <start_sec> <end_sec>   silence the given time range\n"
        << "  gain <factor>                multiply every sample by factor\n"
        << "  mix  <file.wav> <start_sec>  overlay another wav starting at start_sec\n";
}