include_directories(${GTEST_INCLUDE_DIRS} ${INCLUDE_DIRS} ${GLOG_INCLUDE_DIRS})

set(test_sources
    min_heap_test
    bsub_test
)

add_executable(tests ${test_sources})

target_link_libraries(tests
    bsub_static
    ${GTEST_BOTH_LIBRARIES}
    pthread
    ${GLOG_LIBRARIES}
    ${OpenCV_LIBS}
)

add_test(AllUnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests --gtest_shuffle)


# Run the Tests
add_custom_target(
    RunTests ALL
    DEPENDS tests
    COMMAND ctest
)

# Print the Tests
add_custom_target(
    vtest
    DEPENDS tests
    COMMAND ctest -V
)

