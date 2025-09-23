cmake_minimum_required(VERSION 3.31)
project(RyntraProject)

set(CMAKE_CXX_STANDARD 23)

# Project options
option(BUILD_COMPILER "Build Ryntra Compiler" ON)
option(BUILD_TESTS "Build test executables" ON)
option(BUILD_UTILS "Build utility libraries" ON)

# LLVM Configuration (shared across subprojects)
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
    set(LLD_LIBRARY_DIR "${LLVM_BUILD_DIR}/lib")
else ()
    message(STATUS "Attempting to find LLVM in system paths...")
endif ()

if (WIN32)
    add_compile_definitions(NOMINMAX)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

set(LLVM_DIR "${LLVM_DIR}" CACHE PATH "Path to LLVMConfig.cmake")

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM Include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM Libraries: ${LLVM_LIBRARY_DIRS}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(LLVM_LIBS 
    core support irreader bitwriter bitreader
    target x86codegen x86asmparser x86desc x86info
    mc mcparser asmprinter
)

if(LLD_LIBRARY_DIR)
    link_directories(${LLD_LIBRARY_DIR})
endif()

# Add LLD libraries separately (only Common is available)
set(LLD_LIBS
    lldCommon
)

# Add subdirectories
if(BUILD_UTILS)
    add_subdirectory(utils)
endif()

if(BUILD_COMPILER)
    add_subdirectory(compiler)
endif()

# Legacy main executable (for backward compatibility)
if(BUILD_TESTS)
    add_executable(rycc_legacy
        main.cpp
    )
    
    if(TARGET RyntraCompilerLib AND TARGET RyntraUtils)
        target_link_libraries(rycc_legacy 
            RyntraCompilerLib 
            RyntraUtils
            ${LLVM_LIBS}
            ${LLD_LIBS}
        )
        target_include_directories(rycc_legacy PRIVATE .)
    else()
        message(WARNING "RyntraCompilerLib or RyntraUtils not available, skipping rycc_legacy")
    endif()
endif()

# Set default target
if(BUILD_COMPILER)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RyntraCompiler)
elseif(BUILD_RPLINK)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RPLink)
endif()