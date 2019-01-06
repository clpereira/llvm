cmake_minimum_required(VERSION 2.6) 
set(CMAKE_C_COMPILER clang) 
set(CMAKE_CXX_COMPILER clang++)
set(LLVM_SRC_DIR "/Users/cristiano/development/llvm/llvm-7.0.0.src")

# find the llvm-config file
find_program(LLVM_CONFIG "llvm-config")
# set the llvm config command 
# --libs core is missing from the command below, best is to script it so that
# it can be extracted and then added automatically to target_link_libraries
set(LLVM_CXX_FLAGS_CONFIG_COMMAND ${LLVM_CONFIG} "--cxxflags") 
set(LLVM_LD_FLAGS_CONFIG_COMMAND ${LLVM_CONFIG} "--ldflags" "--system-libs" "--libs" "core" "mcjit" "native")

# run llvm_command
execute_process(COMMAND ${LLVM_CXX_FLAGS_CONFIG_COMMAND}
                RESULT_VARIABLE HAD_ERROR
                OUTPUT_VARIABLE CONFIG_OUTPUT)

if (NOT HAD_ERROR)
  # remove all newlines
  string(REPLACE "\n" " " STRIPPED_CONFIG_OUTPUT ${CONFIG_OUTPUT})
  # remove warning configs that cause are unrecognized
  string(REPLACE "-Wno-class-memaccess" " " STRIPPED_CONFIG_OUTPUT ${STRIPPED_CONFIG_OUTPUT})
  # attach output to cxx flags
  set(CMAKE_CXX_FLAGS ${STRIPPED_CONFIG_OUTPUT})
else()
  message(FATAL_ERROR "llvm-config failed with status ${HAD_ERROR}")
endif()

# run llvm_command
execute_process(COMMAND ${LLVM_LD_FLAGS_CONFIG_COMMAND}
                RESULT_VARIABLE HAD_ERROR
                OUTPUT_VARIABLE CONFIG_OUTPUT)

if (NOT HAD_ERROR)
  # attach output to ld flags
  string(REPLACE "\n" " " STRIPPED_CONFIG_OUTPUT ${CONFIG_OUTPUT})
  string(CONCAT STRIPPED_CONFIG_OUTPUT "-rdynamic " ${STRIPPED_CONFIG_OUTPUT})
  set(CMAKE_EXE_LINKER_FLAGS ${STRIPPED_CONFIG_OUTPUT})
else()
  message(FATAL_ERROR "llvm-config failed with status ${HAD_ERROR}")
endif()

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
                    "/usr/local/opt/llvm/include/"
                    "/usr/local/opt/zlib/include/"
                    "${LLVM_SRC_DIR}")

# add the executable
add_executable(kcomp entrypoint.cpp kcomp.cpp ast.cpp lexer.cpp parser.cpp codegen.cpp error.cpp externs.cpp)
#add_executable(kcomp toy.cpp)
#target_link_libraries(kcomp LLVM-6.0)
target_link_libraries(kcomp LLVM) 
