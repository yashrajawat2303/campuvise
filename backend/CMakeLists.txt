cmake_minimum_required(VERSION 3.15)
project(CAMPuVISE LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Install cpp-httplib and nlohmann-json headers through your system package manager.
add_executable(campuvise-server server_main.cpp server.cpp images.cpp)
find_package(Threads REQUIRED)
target_link_libraries(campuvise-server PRIVATE Threads::Threads)
