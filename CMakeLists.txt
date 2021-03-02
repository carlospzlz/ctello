cmake_minimum_required(VERSION 3.10)

project(CTello)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

find_package(spdlog REQUIRED)
find_package(OpenCV REQUIRED)

# CTello Shared Library =======================================================

add_library(ctello SHARED src/ctello.cpp)

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

# CTello Stream ===============================================================

add_executable(ctello-stream src/ctello_stream.cpp)

target_include_directories(ctello-stream PRIVATE include)

target_link_libraries(ctello-stream ctello)
target_link_libraries(ctello-stream ${OpenCV_LIBS})

install(TARGETS ctello-stream DESTINATION bin)

# CTello Joystick =============================================================

add_executable(ctello-joystick src/ctello_joystick.cpp)

target_include_directories(ctello-joystick PRIVATE include)

target_link_libraries(ctello-joystick ctello)

install(TARGETS ctello-stream DESTINATION bin)

# CTello Examples =============================================================

## Flip -----------------------------------------------------------------------
add_executable(flip examples/flip.cpp)

target_include_directories(flip PRIVATE include)

target_link_libraries(flip ctello)
target_link_libraries(flip ${OpenCV_LIBS})

## Follow ---------------------------------------------------------------------
add_executable(follow examples/follow.cpp)

target_include_directories(follow PRIVATE include)

target_link_libraries(follow ctello)
target_link_libraries(follow ${OpenCV_LIBS})
