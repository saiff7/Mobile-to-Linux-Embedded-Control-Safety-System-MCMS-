cmake_minimum_required(VERSION 3.16)
project(MCMS VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(nlohmann_json REQUIRED)

add_executable(mcms
    main.cpp
    server/tcp_server.cpp
    logger/logger.cpp
    engine/control_engine.cpp
    simulator/simulator.cpp
)

target_include_directories(mcms PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/logger
    ${CMAKE_SOURCE_DIR}/simulator
)

target_link_libraries(mcms PRIVATE nlohmann_json::nlohmann_json pthread)
target_compile_options(mcms PRIVATE -Wall -Wextra)
