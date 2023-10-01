cmake_minimum_required(VERSION 3.25)
project(proyectDoom)
set(EXECUTABLE_NAME "proyectDoom")

set(SDL_MAIN_HANDLED ON)
find_package(SDL2 REQUIRED PATHS "C:/Users/danar/Downloads/SDL2-devel-2.28.1-mingw/SDL2")

set(CMAKE_CXX_STANDARD 23)

add_executable(${EXECUTABLE_NAME} main.cpp)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE SDL2::SDL2main)
target_include_directories(${EXECUTABLE_NAME}  PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${EXECUTABLE_NAME}  PRIVATE ${SDL2_LIBRARIES})
include_directories("C:/Users/danar/tmp/glm")
