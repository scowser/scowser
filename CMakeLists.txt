cmake_minimum_required(VERSION 3.22)
project(scowser VERSION 0.0.14 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    WebEngineWidgets
    Network
)

# Source files
set(SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/ui/MainWindow.cpp
    src/ui/TabWidget.cpp
    src/ui/AddressBar.cpp
    src/security/AdBlocker.cpp
    src/security/DnsOverHttps.cpp
    src/security/CertificatePinner.cpp
    src/security/CSPEnforcer.cpp
    src/security/SessionManager.cpp
    src/network/NetworkManager.cpp
    src/network/RequestInterceptor.cpp
    src/sandbox/ProcessSandbox.cpp
)

set(HEADERS
    src/app/Application.h
    src/ui/MainWindow.h
    src/ui/TabWidget.h
    src/ui/AddressBar.h
    src/security/AdBlocker.h
    src/security/DnsOverHttps.h
    src/security/CertificatePinner.h
    src/security/CSPEnforcer.h
    src/security/SessionManager.h
    src/network/NetworkManager.h
    src/network/RequestInterceptor.h
    src/sandbox/ProcessSandbox.h
)

set(RESOURCES
    resources/resources.qrc
)

# macOS .app bundle
if(APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME "scowser")
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.scowser.browser")
    set(MACOSX_BUNDLE_INFO_STRING "scowser - The Secure Browser")
    set(MACOSX_BUNDLE_ICON_FILE "scowser.icns")

    add_executable(scowser MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCES})

    # Copy icon into bundle if it exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/icons/scowser.icns")
        set_source_files_properties(
            "${CMAKE_SOURCE_DIR}/resources/icons/scowser.icns"
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
        )
        target_sources(scowser PRIVATE "${CMAKE_SOURCE_DIR}/resources/icons/scowser.icns")
    endif()
else()
    add_executable(scowser ${SOURCES} ${HEADERS} ${RESOURCES})
endif()

target_include_directories(scowser PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(scowser PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::Network
)

# Platform-specific sandbox libraries
if(APPLE)
    target_link_libraries(scowser PRIVATE "-framework Security" "-framework CoreFoundation")
elseif(UNIX)
    find_package(PkgConfig)
    pkg_check_modules(SECCOMP libseccomp)
    if(SECCOMP_FOUND)
        target_link_libraries(scowser PRIVATE ${SECCOMP_LIBRARIES})
        target_include_directories(scowser PRIVATE ${SECCOMP_INCLUDE_DIRS})
        target_compile_definitions(scowser PRIVATE HAVE_SECCOMP)
    endif()
endif()

# Compiler warnings
target_compile_options(scowser PRIVATE
    -Wall -Wextra -Wpedantic
    -Wno-unused-parameter
)

# Security hardening flags
target_compile_options(scowser PRIVATE
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
)

if(NOT APPLE)
    target_link_options(scowser PRIVATE
        -Wl,-z,relro,-z,now
        -pie
    )
endif()

# Install targets
include(GNUInstallDirs)

if(APPLE)
    install(TARGETS scowser BUNDLE DESTINATION .)
else()
    install(TARGETS scowser RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES packaging/linux/scowser.desktop
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    install(FILES resources/icons/scowser.png
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
            OPTIONAL)
endif()

# CPack for portable archives
set(CPACK_PACKAGE_NAME "scowser")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "scowser - The Secure Browser")
set(CPACK_PACKAGE_VENDOR "scowser")

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "scowser")
else()
    set(CPACK_GENERATOR "TGZ")
endif()
include(CPack)

# Testing
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()
