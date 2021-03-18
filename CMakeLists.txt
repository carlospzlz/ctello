cmake_minimum_required(VERSION 3.10)

project(CTello)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 11)

find_package(spdlog REQUIRED)


# CTello Shared Library =======================================================

add_library(ctello src/ctello.cpp)

target_include_directories(ctello PRIVATE include)

target_link_libraries(ctello PRIVATE spdlog::spdlog)

install(TARGETS ctello DESTINATION lib)
install(FILES include/ctello.h DESTINATION include)

# CTello Command ==============================================================

add_executable(ctello-command src/ctello_command.cpp)

target_include_directories(ctello-command PRIVATE include)

target_link_libraries(ctello-command ctello)

install(TARGETS ctello-command DESTINATION bin)

# CTello State ================================================================

add_executable(ctello-state src/ctello_state.cpp)

target_include_directories(ctello-state PRIVATE include)

target_link_libraries(ctello-state ctello)

install(TARGETS ctello-state DESTINATION bin)

add_executable(example examples/flip.cpp)

target_include_directories(example PRIVATE include)

target_link_libraries(example ctello)