cmake_minimum_required(VERSION 3.22)

project(KiyuExt2)

set(CMAKE_C_STANDARD 99)

include_directories(include)

add_executable(KiyuExt2 src/main.c)

aux_source_directory(./src/model MODEL_SOURCE)
target_sources(KiyuExt2 PRIVATE ${MODEL_SOURCE})

target_compile_definitions(KiyuExt2 PRIVATE $<$<CONFIG:DEBUG>:DEBUG>)
