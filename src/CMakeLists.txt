set(VCROP_SOURCES
    vcrop
)

set(BSUB_SOURCES
    bsub
)

set(SPLITTER_SOURCES
    video_splitter
)

set(BSUB_TEST_SOURCES
    background_subtract
)

include_directories(${INCLUDE_DIRS} ${GLOG_INCLUDE_DIRS})

add_library(vcrop_static STATIC ${VCROP_SOURCES})
add_library(vcrop_shared SHARED ${VCROP_SOURCES})

add_library(bsub_static STATIC ${BSUB_SOURCES})
add_library(bsub_shared SHARED ${BSUB_SOURCES})

add_executable(video_splitter ${SPLITTER_SOURCES})
add_executable(background_subtract ${BSUB_TEST_SOURCES})

target_link_libraries(video_splitter vcrop_shared ${GLOG_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(background_subtract bsub_shared ${GLOG_LIBRARIES} ${OpenCV_LIBS})

