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

# CTello CLI ==================================================================

add_executable(ctello-cli src/ctello_cli.cpp)

target_include_directories(ctello-cli PRIVATE include)

target_link_libraries(ctello-cli ctello)

install(TARGETS ctello-cli DESTINATION bin)

# CTello View =================================================================

add_executable(ctello-view src/ctello_view.cpp)

target_include_directories(ctello-view PRIVATE include)

target_link_libraries(ctello-view ctello)

install(TARGETS ctello-view DESTINATION bin)
