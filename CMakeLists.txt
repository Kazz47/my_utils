cmake_minimum_required(VERSION 2.8.7)
project(K_UTILS)

# Setup C Flags
set(CMAKE_C_FLAGS                   "-Wall")
set(CMAKE_C_FLAGS_DEBUG             "-g")
set(CMAKE_C_FLAGS_MINSIZEREL        "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE           "-O4 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO    "-O2 -g")

# Setup C++ Flags
set(CMAKE_CXX_FLAGS                 "-Wall -std=c++0x")
set(CMAKE_CXX_FLAGS_DEBUG           "-g")
set(CMAKE_CXX_FLAGS_MINSIZEREL      "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE         "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O2 -g")

# Set EXE Linker Flags
set(CMAKE_EXE_LINKER_FLAGS          "-L$ENV{LINKER_LIBRARY_PATH}")
message(STATUS "LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

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
find_package(MySQL REQUIRED)
find_package(Glog REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(OpenCV REQUIRED COMPONENTS videostab)
find_package(BOINC REQUIRED)

message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
message(STATUS "GLOG_LIBRARIES: ${GLOG_LIBRARIES}")
message(STATUS "BOINC_LIBRARIES: ${BOINC_LIBRARIES}")

if(BOINC_FOUND)
    message(STATUS "Found BOINC:  ${BOINC_INCLUDE_DIR}")
endif(BOINC_FOUND)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
)


# Sources
add_subdirectory(src)

# Testing
find_package(GTest REQUIRED)
if(GTEST_FOUND)
    ENABLE_TESTING()
    add_subdirectory(test)
endif(GTEST_FOUND)

