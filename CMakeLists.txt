cmake_minimum_required(VERSION 3.20)
project(ModLoaderABI LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(modloader_abi INTERFACE)
target_include_directories(modloader_abi INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_library(ModLoader::ABI ALIAS modloader_abi)

add_library(modloader_n64_abi INTERFACE)
target_include_directories(modloader_n64_abi INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/n64")
target_link_libraries(modloader_n64_abi INTERFACE ModLoader::ABI)
add_library(ModLoader::N64ABI ALIAS modloader_n64_abi)

add_library(modloader_n64_tools STATIC EXCLUDE_FROM_ALL
    src/n64/assembler.cpp
    src/n64/disassembler.cpp
    src/n64/opcodes.cpp
    src/n64/patch.cpp
    src/n64/relocate.cpp
    src/n64/signature.cpp
)

target_compile_features(modloader_n64_tools PUBLIC cxx_std_17)
target_link_libraries(modloader_n64_tools PUBLIC ModLoader::N64ABI)
add_library(ModLoader::N64Tools ALIAS modloader_n64_tools)
