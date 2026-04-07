find_package(Qt6 REQUIRED COMPONENTS Test Widgets WebEngineWidgets Network)

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

# DNS-over-HTTPS tests
add_executable(test_doh
    test_doh.cpp
    ${CMAKE_SOURCE_DIR}/src/security/DnsOverHttps.cpp
)
target_include_directories(test_doh PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_doh PRIVATE Qt6::Test Qt6::Core Qt6::Network)
add_test(NAME DnsOverHttps COMMAND test_doh)

# Address bar tests
set(CMAKE_AUTORCC ON)
add_executable(test_addressbar
    test_addressbar.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/AddressBar.cpp
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
)
target_include_directories(test_addressbar PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_addressbar PRIVATE Qt6::Test Qt6::Widgets)
add_test(NAME AddressBar COMMAND test_addressbar)
set_tests_properties(AddressBar PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Theme tests
add_executable(test_theme
    test_theme.cpp
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
)
target_include_directories(test_theme PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_theme PRIVATE Qt6::Test Qt6::Widgets)
add_test(NAME Theme COMMAND test_theme)
set_tests_properties(Theme PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Settings tests
add_executable(test_settings
    test_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/app/Settings.cpp
    ${CMAKE_SOURCE_DIR}/src/security/DnsOverHttps.cpp
)
target_include_directories(test_settings PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_settings PRIVATE Qt6::Test Qt6::Core Qt6::Network)
add_test(NAME Settings COMMAND test_settings)

# Settings dialog tests
add_executable(test_settingsdialog
    test_settingsdialog.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/SettingsDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/app/Settings.cpp
    ${CMAKE_SOURCE_DIR}/src/security/DnsOverHttps.cpp
)
target_include_directories(test_settingsdialog PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_settingsdialog PRIVATE Qt6::Test Qt6::Widgets Qt6::Network)
add_test(NAME SettingsDialog COMMAND test_settingsdialog)
set_tests_properties(SettingsDialog PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Download manager tests
add_executable(test_downloadmanager
    test_downloadmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/app/DownloadManager.cpp
)
target_include_directories(test_downloadmanager PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_downloadmanager PRIVATE Qt6::Test Qt6::Core Qt6::WebEngineWidgets)
add_test(NAME DownloadManager COMMAND test_downloadmanager)
set_tests_properties(DownloadManager PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Downloads dialog tests
add_executable(test_downloadsdialog
    test_downloadsdialog.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/DownloadsDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/app/DownloadManager.cpp
)
target_include_directories(test_downloadsdialog PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_downloadsdialog PRIVATE Qt6::Test Qt6::Widgets Qt6::WebEngineWidgets)
add_test(NAME DownloadsDialog COMMAND test_downloadsdialog)
set_tests_properties(DownloadsDialog PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Log panel tests
add_executable(test_logpanel
    test_logpanel.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/LogPanel.cpp
)
target_include_directories(test_logpanel PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_logpanel PRIVATE Qt6::Test Qt6::Widgets)
add_test(NAME LogPanel COMMAND test_logpanel)
set_tests_properties(LogPanel PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Favorites manager tests
add_executable(test_favoritesmanager
    test_favoritesmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/app/FavoritesManager.cpp
)
target_include_directories(test_favoritesmanager PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_favoritesmanager PRIVATE Qt6::Test Qt6::Core)
add_test(NAME FavoritesManager COMMAND test_favoritesmanager)

# Favorites panel tests
add_executable(test_favoritespanel
    test_favoritespanel.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/FavoritesPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/app/FavoritesManager.cpp
)
target_include_directories(test_favoritespanel PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_favoritespanel PRIVATE Qt6::Test Qt6::Widgets)
add_test(NAME FavoritesPanel COMMAND test_favoritespanel)
set_tests_properties(FavoritesPanel PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
