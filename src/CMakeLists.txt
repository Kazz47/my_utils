set(VCROP_SOURCES
    vcrop
)

set(BSUB_SOURCES
    bsub
)

set(KOSUB_SOURCES
    ${BSUB_SOURCES}
    kosub
)

set(VANSUB_SOURCES
    ${BSUB_SOURCES}
    vansub
    video_type
)

set(SPLITTER_SOURCES
    video_splitter
)

set(WILDLIFE_SPLITTER_SOURCES
    wildlife_video_splitter
)

set(BLOB_COUNT_SOURCES
    blob_count
)

set(IMAGE_CROP_SOURCES
    image_crop
)

set(FRACTAL_SOURCES
    fractal
)

set(BSUB_TEST_SOURCES
    background_subtract
)

set(WILDLIFE_BGSUB_SOURCES
    wildlife_bgsub
)

set(EVENT_DATA_PARSER_SOURCES
    event_data_parser

)
set(EVENT_DB_UPLOADER_SOURCES
    event_db_uploader
)

include_directories(${INCLUDE_DIRS} ${GLOG_INCLUDE_DIRS})

add_library(vcrop_static STATIC ${VCROP_SOURCES})
add_library(vcrop_shared SHARED ${VCROP_SOURCES})

add_library(bsub_static STATIC ${BSUB_SOURCES})
add_library(bsub_shared SHARED ${BSUB_SOURCES})

add_library(kosub_static STATIC ${KOSUB_SOURCES})
add_library(kosub_shared SHARED ${KOSUB_SOURCES})

add_library(vansub_static STATIC ${VANSUB_SOURCES})
add_library(vansub_shared SHARED ${VANSUB_SOURCES})

add_executable(video_splitter ${SPLITTER_SOURCES})
add_executable(wildlife_video_splitter ${WILDLIFE_SPLITTER_SOURCES})
add_executable(background_subtract ${BSUB_TEST_SOURCES})
add_executable(wildlife_bgsub ${WILDLIFE_BGSUB_SOURCES})
add_executable(event_data_parser ${EVENT_DATA_PARSER_SOURCES})
add_executable(event_db_uploader ${EVENT_DB_UPLOADER_SOURCES})
add_executable(blob_count ${BLOB_COUNT_SOURCES})
add_executable(image_crop ${IMAGE_CROP_SOURCES})
add_executable(fractal ${FRACTAL_SOURCES})

# Link Shared Libs
target_link_libraries(vcrop_shared ${GLOG_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(bsub_shared ${GLOG_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(kosub_shared ${GLOG_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(vansub_shared ${GLOG_LIBRARIES} ${OpenCV_LIBS})

# Link Executables
target_link_libraries(video_splitter vcrop_shared ${GLOG_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(wildlife_video_splitter vcrop_shared ${GLOG_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(background_subtract bsub_shared kosub_shared vansub_shared ${GLOG_LIBRARIES} ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries(wildlife_bgsub vansub_shared ${GLOG_LIBRARIES} ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries(event_data_parser ${GLOG_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(event_db_uploader ${GLOG_LIBRARIES} ${MYSQL_LIBRARIES})
target_link_libraries(blob_count ${GLOG_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(image_crop ${GLOG_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(fractal ${GLOG_LIBRARIES} ${OpenCV_LIBS})

