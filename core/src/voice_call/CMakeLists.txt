add_library(kipepeo_voice
    voice_engine.cpp
    whisper_stream.cpp
    translator.cpp
    audio_codec.cpp
    p2p_transport.cpp
)

target_include_directories(kipepeo_voice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link against other core libs if needed (e.g. llm, quantization)
# target_link_libraries(kipepeo_voice PRIVATE kipepeo_llm)
