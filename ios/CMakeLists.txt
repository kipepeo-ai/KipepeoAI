cmake_minimum_required(VERSION 3.18)

project(KipepeoKit)

if(NOT APPLE)
    message(FATAL_ERROR "KipepeoKit can only be built on Apple platforms")
endif()

set(CMAKE_SWIFT_LANGUAGE_VERSION 5.0)
enable_language(Swift)

set(KIPEPEOKIT_SOURCES
    KipepeoKit.h
    KipepeoManager.swift
    Interception/KipepeoURLProtocol.swift
    Interception/KipepeoResourceLoaderDelegate.swift
    Interception/AVPlayer+Kipepeo.swift
    Tracking/DataUsageTracker.swift
)

add_library(KipepeoKit SHARED ${KIPEPEOKIT_SOURCES})

# Link with Core libraries
target_link_libraries(KipepeoKit PUBLIC
    kipepeo_core
    kipepeo_video
    kipepeo_kernels
    "-framework Foundation"
    "-framework AVFoundation"
    "-framework CFNetwork"
)

set_target_properties(KipepeoKit PROPERTIES
    FRAMEWORK TRUE
    MACOSX_FRAMEWORK_IDENTIFIER com.kipepeo.kit
    MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    Swift_MODULE_NAME KipepeoKit
)

# Ensure headers are public
set_property(SOURCE KipepeoKit.h PROPERTY MACOSX_PACKAGE_LOCATION Headers)

# --- KipepeoApp ---

set(KIPEPEOAPP_SOURCES
    KipepeoApp/KipepeoApp.swift
    KipepeoApp/ContentView.swift
    KipepeoApp/Views/HomeView.swift
    KipepeoApp/Views/ChatView.swift
    KipepeoApp/Views/CallsView.swift
    KipepeoApp/Views/OnboardingView.swift
    KipepeoApp/Mesh/MeshManager.swift
)


add_executable(KipepeoApp ${KIPEPEOAPP_SOURCES})

target_link_libraries(KipepeoApp PRIVATE KipepeoKit)

set_target_properties(KipepeoApp PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/KipepeoApp/Info.plist
    MACOSX_BUNDLE_GUI_IDENTIFIER com.kipepeo.app
    MACOSX_BUNDLE_BUNDLE_NAME Kipepeo
)

