#project(libShm_v0.1.0)
PROJECT(libShm)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
cmake_minimum_required(VERSION 2.8)
set( BUILD_VERSION 1.2.3.4 CACHE STRING "Version of the product" )
SET(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

if (NOT "$ENV{CPP_PATH}" STREQUAL "")
    set(CPP_PATH "$ENV{CPP_PATH}" CACHE INTERNAL "Copied from environment variable")
endif()
message("CPP_PATH = ${CPP_PATH}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CPP_PATH}/lib)

#find_package (Threads)
#find_package( OpenCV )
file(GLOB SOURCES
    *.h
    *.cpp
)

add_executable(${PROJECT_NAME} "main.cpp" ${SOURCES})
add_library(Shm SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
#${OpenCV_LIBS}
#${CMAKE_THREAD_LIBS_INIT}
#                     )

