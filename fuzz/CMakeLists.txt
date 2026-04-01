cmake_minimum_required(VERSION 3.22)
project(scowser_fuzz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(SRC ${CMAKE_SOURCE_DIR}/../src)

# fuzz_adblocker
add_executable(fuzz_adblocker
    fuzz_adblocker.cpp
    ${SRC}/security/AdBlocker.h
    ${SRC}/security/AdBlocker.cpp
)
target_include_directories(fuzz_adblocker PRIVATE ${SRC})
target_link_libraries(fuzz_adblocker PRIVATE Qt6::Core Qt6::Network)

# fuzz_urlsanitizer (no Q_OBJECT deps)
add_executable(fuzz_urlsanitizer
    fuzz_urlsanitizer.cpp
)
target_include_directories(fuzz_urlsanitizer PRIVATE ${SRC})
target_link_libraries(fuzz_urlsanitizer PRIVATE Qt6::Core Qt6::Network)

# fuzz_csp
add_executable(fuzz_csp
    fuzz_csp.cpp
    ${SRC}/security/CSPEnforcer.h
    ${SRC}/security/CSPEnforcer.cpp
)
target_include_directories(fuzz_csp PRIVATE ${SRC})
target_link_libraries(fuzz_csp PRIVATE Qt6::Core Qt6::Network)

# fuzz_doh
add_executable(fuzz_doh
    fuzz_doh.cpp
    ${SRC}/security/DnsOverHttps.h
    ${SRC}/security/DnsOverHttps.cpp
)
target_include_directories(fuzz_doh PRIVATE ${SRC})
target_link_libraries(fuzz_doh PRIVATE Qt6::Core Qt6::Network)
