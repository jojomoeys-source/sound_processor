cmake_minimum_required(VERSION 3.16)

project(sound_processor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#

# Основное приложение

#

add_executable(sound_processor_1
src/main.cpp
src/application.cpp
src/args_parser.cpp
src/converter.cpp
src/pipeline.cpp
src/waveform.cpp
src/wav_io.cpp
)

target_include_directories(sound_processor_1 PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(sound_processor_1 PRIVATE
-Wall
-Wextra
-Wpedantic
-Werror
)

#

# Тесты

#

include(FetchContent)

FetchContent_Declare(
Catch2
GIT_REPOSITORY https://github.com/catchorg/Catch2.git
GIT_TAG v3.5.3
)

FetchContent_MakeAvailable(Catch2)

enable_testing()

add_executable(sound_processor_tests
sound_processor_tests/test_main.cpp
sound_processor_tests/test_args_parser.cpp
sound_processor_tests/test_filters.cpp
sound_processor_tests/test_pipeline.cpp
sound_processor_tests/test_wav_io.cpp
sound_processor_tests/test_waveform.cpp

```
src/application.cpp
src/args_parser.cpp
src/converter.cpps
src/pipeline.cpp
src/waveform.cpp
src/wav_io.cpp
```

)

target_include_directories(sound_processor_tests PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(sound_processor_tests PRIVATE
Catch2::Catch2WithMain
)

include(CTest)
include(Catch)

catch_discover_tests(sound_processor_tests)
