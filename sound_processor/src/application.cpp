#include "application.h"

#include "filter_producers.h"
#include "wav_io.h"
#include "waveform.h"

#include <iostream>

void Application::configure() {
    // Преобразующие фильтры
    converter_.add_filter_producer("ampl",        filter_producers::make_ampl);
    converter_.add_filter_producer("normalize",   filter_producers::make_normalize);
    converter_.add_filter_producer("silence",     filter_producers::make_silence);
    converter_.add_filter_producer("timestretch", filter_producers::make_timestretch);
    converter_.add_filter_producer("lowpass",     filter_producers::make_lowpass);
    converter_.add_filter_producer("mute",        filter_producers::make_mute);
    converter_.add_filter_producer("gain",        filter_producers::make_gain);
    converter_.add_filter_producer("mix",         filter_producers::make_mix);
    // Генераторы (диспетчер разбирает sin/am/fm внутри)
    converter_.add_filter_producer("generator",   filter_producers::make_generator);
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
        << "Usage: sound_processor [-i <input.wav>] [-o <output.wav>]"
           " [-f <filter> [args...]] ...\n"
        << "\n"
        << "Transforming filters:\n"
        << "  ampl        <factor>                         multiply every sample by factor (factor >= 0)\n"
        << "  normalize   [peak]                           scale so max sample = peak*32767 (default peak=1)\n"
        << "  silence     <sec|ms> <start> <end>           insert silence in [start, end]\n"
        << "  timestretch <factor>                         stretch/compress signal (factor > 0)\n"
        << "  lowpass     <window_size>                    smooth with moving average (odd window)\n"
        << "  mute        <start_sec> <end_sec>            zero out samples in [start, end]\n"
        << "  gain        <factor>                         alias for ampl\n"
        << "  mix         <file.wav> <start_sec>           overlay another wav at start_sec\n"
        << "\n"
        << "Generator filters (ignore input signal):\n"
        << "  generator sin <freq_hz> <dur_ms>                               sine wave\n"
        << "  generator am  <amp> <carrier_hz> <mod_hz> <depth> <dur_ms>    AM signal\n"
        << "  generator fm  <amp> <carrier_hz> <mod_hz> <dev_hz> <dur_ms>   FM signal\n";
}