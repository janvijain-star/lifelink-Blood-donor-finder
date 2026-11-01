cmake_minimum_required(VERSION 3.14)
project(LifeLink CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)

# Crow is header-only: crow_all.h is placed in backend/include/
add_executable(lifelink main.cpp)
target_include_directories(lifelink PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(lifelink PRIVATE SQLite::SQLite3 Threads::Threads)
if (WIN32)
  target_link_libraries(lifelink PRIVATE ws2_32 wsock32)
endif()
