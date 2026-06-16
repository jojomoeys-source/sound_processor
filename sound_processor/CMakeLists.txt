cmake_minimum_required(VERSION 3.16)
project(sound_processor CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Основное приложение ──────────────────────────────────────────────────────

add_executable(sound_processor
    main.cpp
    application.cpp
    args_parser.cpp
    converter.cpp
    pipeline.cpp
    waveform.cpp
    wav_io.cpp
)

target_include_directories(sound_processor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(sound_processor PRIVATE
    -Wall -Wextra -Wpedantic -Werror
)

# ─── Тесты ───────────────────────────────────────────────────────────────────

# Подключаем Catch2 через FetchContent (не требует ручной установки)
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.3
)
FetchContent_MakeAvailable(Catch2)

add_executable(sound_processor_tests
    tests/test_main.cpp
    tests/test_waveform.cpp
    tests/test_args_parser.cpp
    tests/test_filters.cpp
    tests/test_pipeline.cpp
    tests/test_wav_io.cpp
    # Компоненты приложения (без main.cpp)
    args_parser.cpp
    converter.cpp
    pipeline.cpp
    waveform.cpp
    wav_io.cpp
)

target_include_directories(sound_processor_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(sound_processor_tests PRIVATE Catch2::Catch2WithMain)

target_compile_options(sound_processor_tests PRIVATE
    -Wall -Wextra -Wpedantic
)

include(CTest)
include(Catch)
catch_discover_tests(sound_processor_tests)