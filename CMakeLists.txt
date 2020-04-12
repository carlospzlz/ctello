cmake_minimum_required(VERSION 3.10)

project(CTello)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

find_package(spdlog REQUIRED)

# CTello Shared Library =======================================================

add_library(ctello SHARED src/ctello.cpp)

target_include_directories(ctello PRIVATE include)

target_link_libraries(ctello PRIVATE spdlog::spdlog)

install(TARGETS ctello DESTINATION lib)
install(FILES include/ctello.h DESTINATION include)

# CTello Command ==============================================================

add_executable(ctello-cli src/ctello_cli.cpp)

target_include_directories(ctello-cli PRIVATE include)

target_link_libraries(ctello-cli ctello)

install(TARGETS ctello-cli DESTINATION bin)

# CTello Status ===============================================================

add_executable(ctello-status src/ctello_status.cpp)

target_include_directories(ctello-status PRIVATE include)

target_link_libraries(ctello-status ctello)

install(TARGETS ctello-status DESTINATION bin)

# CTello Stream ===============================================================

add_executable(ctello-stream src/ctello_stream.cpp)

target_include_directories(ctello-stream PRIVATE include)

target_link_libraries(ctello-stream ctello)

install(TARGETS ctello-stream DESTINATION bin)
