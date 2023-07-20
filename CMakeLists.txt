cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)
project("Falling_Sand" C CXX)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(PROJECT_ORIGIN "./")

file(GLOB SOURCE
    "src/*.h"
    "src/*.cpp"
)
set(SOURCE ${SOURCE} ./glad.c)
set(NAME "falling_sand")

find_package(OpenMP REQUIRED)
link_directories(${PROJECT_ORIGIN}/lib)

if(WIN32)
  set(LIBS glfw3 opengl32 freetype)
  set(CMAKE_CXX_STANDARD 20)
  list(APPEND CMAKE_CXX_FLAGS "-std=c++20")
endif(WIN32)

if(UNIX)
  find_package(PkgConfig REQUIRED)
  pkg_search_module(GLFW REQUIRED glfw3)
  find_package(glm REQUIRED)
  list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -Ofast")
  # list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -O0 -g")
  set(LIBS ${GLFW_STATIC_LIBRARIES} freetype)
  include_directories(${GLFW_INCLUDE_DIRS})
  include_directories(${GLM_INCLUDE_DIRS})
endif(UNIX)

add_executable(${NAME} ${SOURCE})
target_link_libraries(${NAME} ${LIBS} OpenMP::OpenMP_CXX)
include_directories(${PROJECT_ORIGIN}/includes)