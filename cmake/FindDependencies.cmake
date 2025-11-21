# FindDependencies.cmake
# Utility functions for finding and configuring third-party dependencies

# Find llama.cpp
function(find_llama_cpp)
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/llama.cpp/CMakeLists.txt")
        add_subdirectory(third_party/llama.cpp)
        set(LLAMA_CPP_FOUND TRUE PARENT_SCOPE)
        message(STATUS "Found llama.cpp in third_party/")
    else()
        message(WARNING "llama.cpp not found in third_party/. Please add it as a submodule or fork.")
        set(LLAMA_CPP_FOUND FALSE PARENT_SCOPE)
    endif()
endfunction()

# Find rav1e
function(find_rav1e)
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/rav1e/Cargo.toml")
        # rav1e is a Rust project, will need special handling
        message(STATUS "rav1e found in third_party/ (Rust project - requires special integration)")
        set(RAV1E_FOUND TRUE PARENT_SCOPE)
    else()
        message(WARNING "rav1e not found in third_party/. Please add it as a submodule or fork.")
        set(RAV1E_FOUND FALSE PARENT_SCOPE)
    endif()
endfunction()

# Find dav1d
function(find_dav1d)
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/dav1d/meson.build")
        # dav1d uses meson build system, will need special handling
        message(STATUS "dav1d found in third_party/ (Meson project - requires special integration)")
        set(DAV1D_FOUND TRUE PARENT_SCOPE)
    else()
        message(WARNING "dav1d not found in third_party/. Please add it as a submodule or fork.")
        set(DAV1D_FOUND FALSE PARENT_SCOPE)
    endif()
endfunction()

# Find whisper.cpp
function(find_whisper_cpp)
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/whisper.cpp/CMakeLists.txt")
        add_subdirectory(third_party/whisper.cpp)
        set(WHISPER_CPP_FOUND TRUE PARENT_SCOPE)
        message(STATUS "Found whisper.cpp in third_party/")
    else()
        message(WARNING "whisper.cpp not found in third_party/. Please add it as a submodule or fork.")
        set(WHISPER_CPP_FOUND FALSE PARENT_SCOPE)
    endif()
endfunction()

# Check all dependencies
function(check_all_dependencies)
    message(STATUS "Checking third-party dependencies...")
    find_llama_cpp()
    find_rav1e()
    find_dav1d()
    find_whisper_cpp()
endfunction()

