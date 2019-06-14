cmake_minimum_required(VERSION 3.14)
project(WebAudioOpenAL)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "--post-js ${PROJECT_SOURCE_DIR}/EmPost.js --bind -s WASM=1")

add_executable(${CMAKE_PROJECT_NAME} main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE openal)

