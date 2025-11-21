add_library(kipepeo_health
    health_engine.cpp
)

target_include_directories(kipepeo_health PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
