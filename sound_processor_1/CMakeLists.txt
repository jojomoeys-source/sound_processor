cmake_minimum_required(VERSION 3.16)

project(sound_processor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOUND_PROCESSOR_SOURCES
    src/app/application.cpp
    src/args/args_parser.cpp
    src/converter/converter.cpp
    src/pipeline/pipeline.cpp
    src/waveform/waveform.cpp
    src/wav/wav_io.cpp
)

set(SOUND_PROCESSOR_HEADERS
    src/app/application.h
    src/args/args_parser.h
    src/converter/converter.h
    src/filter/filter.h
    src/filter_producers/filter_producers.h
    src/filters/ampl_filter.h
    src/filters/normalize_filter.h
    src/filters/silence_filter.h
    src/filters/timestretch_filter.h
    src/filters/lowpass_filter.h
    src/filters/highpass_filter.h
    src/filters/bandpass_filter.h
    src/filters/reject_filter.h
    src/filters/generator_filters.h
    src/filters/mute_filter.h
    src/filters/mix_filter.h
    src/pipeline/pipeline.h
    src/wav/wav_io.h
    src/wav/wav_headers.h
    src/waveform/waveform.h
)

add_executable(sound_processor
    ${SOUND_PROCESSOR_SOURCES}
    ${SOUND_PROCESSOR_HEADERS}
    src/app/main.cpp
)

target_include_directories(sound_processor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(sound_processor PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

include(CTest)

if(BUILD_TESTING)
    find_package(Catch2 3 QUIET)
    if(NOT Catch2_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            Catch2
            URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.3.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
        set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(Catch2)
    endif()

    add_executable(sound_processor_tests
        ${SOUND_PROCESSOR_SOURCES}
        ${SOUND_PROCESSOR_HEADERS}
        tests/sound_processor_tests/test_args_parser.cpp
        tests/sound_processor_tests/test_waveform.cpp
        tests/sound_processor_tests/test_filters.cpp
        tests/sound_processor_tests/test_pipeline.cpp
        tests/sound_processor_tests/test_wav_io.cpp
    )

    target_include_directories(sound_processor_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(sound_processor_tests PRIVATE
        Catch2::Catch2WithMain
    )

    target_compile_options(sound_processor_tests PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    add_test(NAME sound_processor_tests COMMAND sound_processor_tests)
endif()
