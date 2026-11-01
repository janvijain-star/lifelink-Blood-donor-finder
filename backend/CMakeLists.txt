cmake_minimum_required(VERSION 3.14)
project(LifeLink CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# Crow is header-only: crow_all.h is placed in backend/include/
add_executable(lifelink_server server.cpp db_manager.cpp sqlite3.c)
target_include_directories(lifelink_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lifelink_server PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
if (WIN32)
  target_link_libraries(lifelink_server PRIVATE ws2_32 mswsock)
endif()
