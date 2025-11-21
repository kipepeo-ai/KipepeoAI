add_library(kipepeo_finance
    finance_engine.cpp
)

target_include_directories(kipepeo_finance PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
