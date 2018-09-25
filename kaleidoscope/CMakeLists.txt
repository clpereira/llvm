cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

find_program(LLVM_CONFIG "llvm-config")
set(LLVM_CONFIG_COMMAND ${LLVM_CONFIG} "--cxxflags")
execute_process(COMMAND ${LLVM_CONFIG_COMMAND}
                RESULT_VARIABLE HAD_ERROR
                OUTPUT_VARIABLE CONFIG_OUTPUT)
set(CMAKE_CXX_FLAGS ${OUTPUT_VARIABLE})

project(kcomp)

# add a version number
set(kcomp_VERSION_MAJOR 1)
set(kcomp_VERSION_MINOR 0)

# configure a header file to pass some of the Cmake settings
# to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/kcomp_config.h.in"
  "${PROJECT_BINARY_DIR}/kcomp_config.h"
  )

# add the binary tree to the search path for include files
# so that it will find kcomp_config.h
include_directories("${PROJECT_BINARY_DIR}"
                    "/usr/include/llvm-6.0")

# add the executable
add_executable(kcomp entrypoint.cpp lexer.cpp parser.cpp)
