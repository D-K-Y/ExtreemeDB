cmake_minimum_required(VERSION 3.16)
project(InMemoryPLSQLDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/database/storage_engine.cpp
    src/database/table.cpp
    src/database/index.cpp
    src/plsql/lexer.cpp
    src/plsql/parser.cpp
    src/plsql/executor.cpp
    src/query/query_processor.cpp
    src/utils/logger.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} Threads::Threads)