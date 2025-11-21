# Core Libraries CMakeLists.txt

if(KIPEPEO_BUILD_LLM)
    add_subdirectory(llm)
endif()

if(KIPEPEO_BUILD_VIDEO)
    add_subdirectory(video)
endif()

if(KIPEPEO_BUILD_KERNELS)
    add_subdirectory(kernels)
endif()

if(KIPEPEO_BUILD_QUANTIZATION)
    add_subdirectory(quantization)
endif()

