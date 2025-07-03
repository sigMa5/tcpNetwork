cmake_minimum_required(VERSION 3.15)
project(async_router LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio)        # asio in Boost 1.66+
find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(server
  server.cpp
)

target_link_libraries(server
  PRIVATE
    Threads::Threads
    Boost::asio
    nlohmann_json::nlohmann_json
)

if (WIN32)
  target_link_libraries(server PRIVATE Ws2_32)
endif()
