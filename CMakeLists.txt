cmake_minimum_required(VERSION 3.31)
project(RyntraProj)

set(CMAKE_CXX_STANDARD 23)

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

set(COMPILER_SOURCES
        compiler/compiler.cpp
)

# Main executable
add_executable(RyntraProj
        main.cpp
        ${LEXER_SOURCES}
        ${PARSER_SOURCES}
        ${COMPILER_SOURCES}
)

# Test executable
add_executable(RyntraCompilerTest
        compiler/test/test_compiler.cpp
        ${LEXER_SOURCES}
        ${PARSER_SOURCES}
        ${COMPILER_SOURCES}
)

llvm_map_components_to_libnames(LLVM_LIBS core support irreader)

# Include directories
target_include_directories(RyntraProj PRIVATE compiler)
target_include_directories(RyntraCompilerTest PRIVATE compiler)

target_link_libraries(RyntraProj ${LLVM_LIBS})
target_link_libraries(RyntraCompilerTest ${LLVM_LIBS})