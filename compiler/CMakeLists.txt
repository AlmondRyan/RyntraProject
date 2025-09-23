cmake_minimum_required(VERSION 3.31)
project(RyntraCompiler)

set(CMAKE_CXX_STANDARD 23)

if(NOT LLVM_FOUND)
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
        message(STATUS "Attempting to find LLVM in system paths...")
    endif ()

    set(LLVM_DIR "${LLVM_DIR}" CACHE PATH "Path to LLVMConfig.cmake")

    find_package(LLVM REQUIRED CONFIG)

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "LLVM Include dirs: ${LLVM_INCLUDE_DIRS}")
    message(STATUS "LLVM Libraries: ${LLVM_LIBRARY_DIRS}")

    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})

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
    
    set(LLD_LIBS
        lldCommon
    )
endif()

# Compiler source files
set(LEXER_SOURCES
    lexer/token.cpp
    lexer/lexer.cpp
)

set(PARSER_SOURCES
    parser/ast.cpp
    parser/parser.cpp
)

set(CODEGEN_SOURCES
    codegen/llvm_codegen.cpp
    codegen/integrated_linker.cpp
)

set(COMPILER_CORE_SOURCES
    compiler.cpp
)

# Create RyntraCompiler library
add_library(RyntraCompilerLib STATIC
    ${LEXER_SOURCES}
    ${PARSER_SOURCES}
    ${CODEGEN_SOURCES}
    ${COMPILER_CORE_SOURCES}
)

# Set target properties
target_include_directories(RyntraCompilerLib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LLVM_INCLUDE_DIRS}
)

target_link_libraries(RyntraCompilerLib PUBLIC ${LLVM_LIBS} ${LLD_LIBS})

# Create RyntraCompiler executable (CLI)
add_executable(RyntraCompiler
    main_cli.cpp
        ../utils/ErrorHandler.h
        ../utils/ConsoleTextColorManager.h
)

target_link_libraries(RyntraCompiler 
    RyntraCompilerLib
    ${LLVM_LIBS}
    ${LLD_LIBS}
)

target_include_directories(RyntraCompiler PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../utils")
    target_sources(RyntraCompiler PRIVATE
        ../utils/arg_parser.cpp
        ../utils/compiler_utils.cpp
        ../utils/ErrorHandler.cpp
    )
endif()

install(TARGETS RyntraCompiler RyntraCompilerLib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY lexer/ parser/ codegen/
    DESTINATION include/ryntra/compiler
    FILES_MATCHING PATTERN "*.h"
)

install(FILES compiler.h
    DESTINATION include/ryntra/compiler
)