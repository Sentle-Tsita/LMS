cmake_minimum_required(VERSION 3.14)
project(LMS LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LMS_BUILD_TESTS "Build the GoogleTest tests" ON)
option(LMS_WERROR      "Treat compiler warnings as errors" OFF)
option(LMS_COVERAGE    "Build with gcov coverage instrumentation" OFF)

# SQLite library (public-domain amalgamation, no warning flags: it is third-party code)
add_library(sqlite_lib third_party/sqlite/sqlite3.c)
target_include_directories(sqlite_lib PUBLIC third_party/sqlite)

# LMS core library: everything except main()
add_library(lms_core
    src/data/Database.cpp
    src/data/Statement.cpp
    src/security/PasswordHasher.cpp
    src/storage/FileStorage.cpp
    src/services/AccountService.cpp
    src/services/AssignmentService.cpp
    src/services/SubmissionService.cpp
    src/services/ModerationService.cpp
    src/ui/ConsoleApp.cpp
    src/ui/DemoData.cpp
)
target_include_directories(lms_core PUBLIC src)
target_link_libraries(lms_core PUBLIC sqlite_lib)
target_compile_definitions(lms_core PUBLIC LMS_SCHEMA_PATH="${CMAKE_SOURCE_DIR}/sql/schema.sql")
target_compile_options(lms_core PUBLIC -Wall -Wextra -Wpedantic)
if(LMS_WERROR)
    target_compile_options(lms_core PUBLIC -Werror)
endif()
if(LMS_COVERAGE)
    target_compile_options(lms_core PUBLIC --coverage -O0 -g)
    target_link_libraries(lms_core PUBLIC --coverage)
endif()

# Password hashing library: Windows CNG on Windows, OpenSSL elsewhere
if(WIN32)
    target_link_libraries(lms_core PUBLIC bcrypt)
else()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(lms_core PUBLIC OpenSSL::Crypto)
endif()

# Program
add_executable(lms src/main.cpp)
target_link_libraries(lms PRIVATE lms_core)

# Tests
if(LMS_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_executable(lms_tests
        tests/test_datetime.cpp
        tests/test_penalty.cpp
        tests/test_password.cpp
        tests/test_file_storage.cpp
        tests/test_schema.cpp
        tests/test_repository.cpp
        tests/test_services.cpp
        tests/test_system.cpp
        tests/test_nonfunctional.cpp
    )
    target_link_libraries(lms_tests PRIVATE lms_core GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(lms_tests)
endif()

install(TARGETS lms
    RUNTIME DESTINATION .
)

install(FILES
    README.md
    CODING_STANDARD.md
    DESTINATION .
)

install(FILES
    packaging/start-lms.bat
    packaging/start-lms-demo.bat
    DESTINATION .
)

install(FILES
    sql/schema.sql
    DESTINATION sql
)

# ZIP package configuration
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "LMS")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_FILE_NAME "LMS-v1.0")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/packages")

include(CPack)

# Include MinGW runtime DLLs in the release package
get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

install(FILES
    "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
    "${MINGW_BIN_DIR}/libstdc++-6.dll"
    "${MINGW_BIN_DIR}/libwinpthread-1.dll"
    DESTINATION .
)
