cmake_minimum_required(VERSION 3.10)

# Project name
project(Primordial
    VERSION 0.1.0
    DESCRIPTION ""
    LANGUAGES C CXX
)

# Find Lyra for command line arguments
#find_package(Lyra CONFIG REQUIRED)

find_package(glm CONFIG REQUIRED)

option(USE_TERRA_SUBPROJECT "" ON)
if (USE_TERRA_SUBPROJECT)
    set(Terra_DIR "" CACHE PATH "")
    set(Terra_BIN_DIR "" CACHE PATH "")
    add_subdirectory(${Terra_DIR} ${Terra_BIN_DIR})
else()
    # Find Terra
    find_package(Terra CONFIG REQUIRED)
endif()

# Do stuff depending on the compiler
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Wpedantic -Wunused-value -Wold-style-cast")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "/W4")
    set(CMAKE_CXX_FLAGS_DEBUG "/O0 /ZI")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(prmrdl
    src/main.cpp
)

set_target_properties(prmrdl
    PROPERTIES 
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

target_link_libraries(prmrdl PRIVATE glm Terra)

terra_copy_kernels(prmrdl)
