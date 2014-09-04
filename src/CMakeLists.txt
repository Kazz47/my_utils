set(SOURCES
    heap
    min_heap
)

include_directories(${INCLUDE_DIRS})

# add_library(slog OBJECT ${SOURCES})
# add_library(slog_static STATIC $<TARGET_OBJECTS:soundex>)
# add_library(slog_shared SHARED $<TARGET_OBJECTS:soundex>)

add_library(heap_static STATIC ${SOURCES})
add_library(heap_shared SHARED ${SOURCES})

