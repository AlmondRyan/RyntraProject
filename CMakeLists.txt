cmake_minimum_required(VERSION 3.31)
project(RyntraProj)

set(CMAKE_CXX_STANDARD 23)

# LLVM Configuration
# 更好的LLVM查找逻辑
if (DEFINED ENV{LLVM_BUILD})
    set(LLVM_BUILD_DIR "$ENV{LLVM_BUILD}")
    message(STATUS "Using LLVM_BUILD environment variable: ${LLVM_BUILD_DIR}")
elseif (DEFINED ENV{LLVM_HOME})
    set(LLVM_BUILD_DIR "$ENV{LLVM_HOME}")
    message(STATUS "Using LLVM_HOME environment variable: ${LLVM_BUILD_DIR}")
else ()
    message(WARNING "Environment Variables LLVM_BUILD and LLVM_HOME are not set.")
endif ()

if (LLVM_BUILD_DIR)
    set(LLVM_DIR "${LLVM_BUILD_DIR}/lib/cmake/llvm")
    message(STATUS "LLVM_DIR set to: ${LLVM_DIR}")
else ()
    # 如果没有设置环境变量，让find_package尝试系统默认路径
    message(STATUS "Attempting to find LLVM in system paths...")
endif ()

# 将LLVM_DIR设为缓存变量，允许命令行覆盖
set(LLVM_DIR "${LLVM_DIR}" CACHE PATH "Path to LLVMConfig.cmake")

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM Include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM Libraries: ${LLVM_LIBRARY_DIRS}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Compiler source files
set(LEXER_SOURCES
        compiler/lexer/token.cpp
        compiler/lexer/lexer.cpp
)

set(PARSER_SOURCES
        compiler/parser/ast.cpp
        compiler/parser/parser.cpp
)

set(CODEGEN_SOURCES
        compiler/codegen/llvm_codegen.cpp
)

set(COMPILER_SOURCES
        compiler/compiler.cpp
)

set(UTILS_SOURCES
        utils/arg_parser.cpp
        utils/compiler_utils.cpp
)

# Main executable (rycc - Ryntra Compiler)
add_executable(rycc
        main.cpp
        ${LEXER_SOURCES}
        ${PARSER_SOURCES}
        ${CODEGEN_SOURCES}
        ${COMPILER_SOURCES}
        ${UTILS_SOURCES}
)

# Test executable
add_executable(RyntraCompilerTest
        compiler/test/test_compiler.cpp
        ${LEXER_SOURCES}
        ${PARSER_SOURCES}
        ${CODEGEN_SOURCES}
        ${COMPILER_SOURCES}
)

# LLVM Code Generation test executable
add_executable(RyntraLLVMTest
        compiler/test/test_llvm_codegen.cpp
        ${LEXER_SOURCES}
        ${PARSER_SOURCES}
        ${CODEGEN_SOURCES}
        ${COMPILER_SOURCES}
)

# Full Compilation Pipeline test executable
add_executable(RyntraFullCompileTest
        compiler/test/test_full_compilation.cpp
        ${LEXER_SOURCES}
        ${PARSER_SOURCES}
        ${CODEGEN_SOURCES}
        ${COMPILER_SOURCES}
)

# Map LLVM components to libraries
llvm_map_components_to_libnames(LLVM_LIBS 
    core 
    support 
    irreader 
    bitwriter 
    bitreader
    target
    x86codegen
    x86asmparser
    x86desc
    x86info
    mc
    mcparser
    asmprinter
)

# Include directories
target_include_directories(rycc PRIVATE .)
target_include_directories(RyntraCompilerTest PRIVATE .)
target_include_directories(RyntraLLVMTest PRIVATE .)
target_include_directories(RyntraFullCompileTest PRIVATE .)

target_link_libraries(rycc ${LLVM_LIBS})
target_link_libraries(RyntraCompilerTest ${LLVM_LIBS})
target_link_libraries(RyntraLLVMTest ${LLVM_LIBS})
target_link_libraries(RyntraFullCompileTest ${LLVM_LIBS})