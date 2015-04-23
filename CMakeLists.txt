cmake_minimum_required(VERSION 2.8.7)
project(K_UTILS)

# Setup C Flags
set(CMAKE_C_FLAGS                   "-Wall -Wextra -pedantic")
set(CMAKE_C_FLAGS_DEBUG             "-g")
set(CMAKE_C_FLAGS_MINSIZEREL        "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE           "-O4 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO    "-O2 -g")

# Setup C++ Flags
set(CMAKE_CXX_FLAGS                 "-Wall -Wextra -std=c++0x -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG           "-g")
set(CMAKE_CXX_FLAGS_MINSIZEREL      "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE         "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O2 -g")

# Set EXE Linker Flags
if(NOT $ENV{LINKER_LIBRARY_PATH} STREQUAL "")
    set(CMAKE_EXE_LINKER_FLAGS          "-L$ENV{LINKER_LIBRARY_PATH}")
    message(STATUS "LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
endif(NOT $ENV{LINKER_LIBRARY_PATH} STREQUAL "")

# Look for module files in these locations
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${PROJECT_SOURCE_DIR}/cmake_modules/"
)

# Set Apple RPATH
if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_SKIP_BUILD_RPATH 0)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.9")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.9")
endif(APPLE)

if(MSVC)
    set(OM_USE_LIBCMT 1) #Look for Windows lib names
    set(GLOG_ROOT_DIR "$ENV{HOMEPATH}/repos/glog")
    set(BOOST_ROOT "$ENV{HOMEPATH}/boost/boost_1_58_0")
    set(OpenCV_DIR "$ENV{HOMEPATH}/installs")
endif(MSVC)

# Setup Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
    )
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

# Setting Vars
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED OFF)
set(OpenCV_STATIC ON)

# Find Packages
find_package(Threads REQUIRED)
find_package(MySQL REQUIRED)
find_package(Glog REQUIRED)
if(MSVC)
    find_package(Boost REQUIRED)
else(MSVC)
    find_package(Boost REQUIRED COMPONENTS system filesystem)
endif(MSVC)
find_package(OpenCV REQUIRED COMPONENTS videostab)
find_package(BOINC REQUIRED)

if(GLOG_FOUND)
    include_directories(${GLOG_INCLUDE_DIRS})
endif(GLOG_FOUND)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif(Boost_FOUND)

if(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif(OpenCV_FOUND)

if(BOINC_FOUND)
    include_directories(${BOINC_INCLUDE_DIRS})
endif(BOINC_FOUND)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Sources
add_subdirectory(src)

# Testing
find_package(GTest)
if(GTEST_FOUND)
    ENABLE_TESTING()
    add_subdirectory(test)
endif(GTEST_FOUND)

