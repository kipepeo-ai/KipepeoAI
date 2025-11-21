add_library(kipepeo_vision
    vision_engine.cpp
    clip_model.cpp
    sd_model.cpp
    llava_model.cpp
)

target_include_directories(kipepeo_vision PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
