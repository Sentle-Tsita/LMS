cmake_minimum_required(VERSION 3.14)
project(LMS LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SQLite library
add_library(sqlite_lib third_party/sqlite/sqlite3.c)
target_include_directories(sqlite_lib PUBLIC third_party/sqlite)

# LMS core library
add_library(lms_core src/data/Database.cpp src/data/Statement.cpp)
target_include_directories(lms_core PUBLIC src)
target_link_libraries(lms_core PUBLIC sqlite_lib)
target_compile_definitions(lms_core PUBLIC
    LMS_SCHEMA_PATH="${CMAKE_SOURCE_DIR}/sql/schema.sql"
)

# LMS executable
add_executable(lms src/main.cpp)
target_link_libraries(lms PRIVATE lms_core)