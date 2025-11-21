# Tests CMakeLists.txt

if(KIPEPEO_BUILD_TESTS)
    enable_testing()
    
    # Unit tests
    add_subdirectory(unit)
    
    # Integration tests
    add_subdirectory(integration)
endif()

