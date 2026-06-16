cmake_minimum_required(VERSION 3.16)

project(sound_processor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sound_processor
src/main.cpp
src/application.cpp
src/args_parser.cpp
src/converter.cpp
src/pipeline.cpp
src/waveform.cpp
src/wav_io.cpp
)

target_include_directories(sound_processor PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(sound_processor PRIVATE
-Wall
-Wextra
-Wpedantic
)

