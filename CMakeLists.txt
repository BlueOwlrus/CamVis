
cmake_minimum_required(VERSION 3.4)
project(CamVis)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DGLEW_STATIC")

include_directories("C:/GLFW/include" "C:/GLEW/include" "C:/GLM" "C:/GLFW/other_include")
link_directories("C:/GLFW/lib-mingw" "C:/GLEW/lib/Release/x64" "C:/GLFW/other_libs")

set(SOURCE_FILES main.cpp AFace.cpp AFace.h AVertex.cpp AVertex.h AMesh.cpp AMesh.h)
add_executable(CamVis ${SOURCE_FILES})

target_link_libraries(CamVis glfw3 glew32s glu32 opengl32)