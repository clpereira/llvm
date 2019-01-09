cmake_minimum_required(VERSION 3.4.3)
set(CMAKE_C_COMPILER clang)  
set(CMAKE_CXX_COMPILER clang++) 
project(kcomp)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

# add a version number
set(kcomp_VERSION_MAJOR 1)
set(kcomp_VERSION_MINOR 0)

# configure a header file to pass some of the Cmake settings
# to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/kcomp_config.h.in"
  "${PROJECT_BINARY_DIR}/kcomp_config.h"
  )

if (NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

set(LLVM_SRC_DIR "/home/cristiano/development/llvm/llvm-7.0.1.src")
include_directories(${LLVM_INCLUDE_DIRS}
                    ${LLVM_SRC_DIR})

add_definitions(${LLVM_DEFINITIONS})
set(CMAKE_EXE_LINKER_FLAGS "-Wl-export-dynamic")

# Now build our tools
add_executable(kcomp entrypoint.cpp kcomp.cpp ast.cpp lexer.cpp parser.cpp codegen.cpp error.cpp externs.cpp)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs analysis core executionengine instcombine object orcjit runtimedyld scalaropts support native x86asmprinter x86asmparser)

# Link against LLVM libraries
target_link_libraries(kcomp ${llvm_libs}
                             rt
                             dl
                             tinfo
                             pthread
                             m)

