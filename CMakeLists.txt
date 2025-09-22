cmake_minimum_required(VERSION 3.31)
project(RyntraProj)

set(CMAKE_CXX_STANDARD 23)

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

# Lexer test executable
add_executable(RyntraLexerTest
    compiler/test/test_lexer.cpp
    ${LEXER_SOURCES}
)

# Method parsing test executable
add_executable(RyntraMethodTest
    compiler/test/test_method_parsing.cpp
    ${LEXER_SOURCES}
    ${PARSER_SOURCES}
    ${COMPILER_SOURCES}
)

# Namespace test executable
add_executable(RyntraNamespaceTest
    compiler/test/test_namespace.cpp
    ${LEXER_SOURCES}
    ${PARSER_SOURCES}
    ${COMPILER_SOURCES}
)

# Include directories
target_include_directories(RyntraProj PRIVATE compiler)
target_include_directories(RyntraCompilerTest PRIVATE compiler)
target_include_directories(RyntraLexerTest PRIVATE compiler)
target_include_directories(RyntraMethodTest PRIVATE compiler)
target_include_directories(RyntraNamespaceTest PRIVATE compiler)
