set(VCROP_SOURCES
    vcrop
)

set(SOURCES
    video_splitter
)

include_directories(${INCLUDE_DIRS} ${GLOG_INCLUDE_DIRS})

add_library(vcrop_static STATIC ${SOURCES})
add_library(vcrop_shared SHARED ${SOURCES})

add_executable(video_splitter ${SOURCES})

target_link_libraries(video_splitter ${GLOG_LIBRARIES} ${OpenCV_LIBS})

