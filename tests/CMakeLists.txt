find_package(Qt6 REQUIRED COMPONENTS Test WebEngineWidgets Network)

# Ad blocker tests
add_executable(test_adblocker
    test_adblocker.cpp
    ${CMAKE_SOURCE_DIR}/src/security/AdBlocker.cpp
)
target_include_directories(test_adblocker PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_adblocker PRIVATE Qt6::Test Qt6::Core Qt6::Network)
add_test(NAME AdBlocker COMMAND test_adblocker)

# Certificate pinner tests
add_executable(test_certpinner
    test_certpinner.cpp
    ${CMAKE_SOURCE_DIR}/src/security/CertificatePinner.cpp
)
target_include_directories(test_certpinner PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_certpinner PRIVATE Qt6::Test Qt6::Core Qt6::Network)
add_test(NAME CertificatePinner COMMAND test_certpinner)

# CSP enforcer tests
add_executable(test_csp
    test_csp.cpp
    ${CMAKE_SOURCE_DIR}/src/security/CSPEnforcer.cpp
)
target_include_directories(test_csp PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_csp PRIVATE Qt6::Test Qt6::Core)
add_test(NAME CSPEnforcer COMMAND test_csp)
