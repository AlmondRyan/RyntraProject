cmake_minimum_required(VERSION 3.31)
project(RyntraUtils)

set(CMAKE_CXX_STANDARD 23)

# Utils source files
set(UTILS_SOURCES
    ErrorHandler.cpp
    arg_parser.cpp
    compiler_utils.cpp
)

# Create RyntraUtils library
add_library(RyntraUtils STATIC
    ${UTILS_SOURCES}
)

# Set target properties
target_include_directories(RyntraUtils PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Install targets
install(TARGETS RyntraUtils
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install headers
install(FILES 
    ErrorHandler.h
    ConsoleTextColorManager.h
    arg_parser.h
    compiler_utils.h
    DESTINATION include/ryntra/utils
)