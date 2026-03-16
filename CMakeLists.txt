cmake_minimum_required(VERSION 3.14)
project(AirTravelDB LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(airtraveldb
    src/main.cpp
    src/DataManager.cpp
)

target_include_directories(airtraveldb PRIVATE include)

if(WIN32)
    target_link_libraries(airtraveldb PRIVATE ws2_32)
    target_compile_definitions(airtraveldb PRIVATE
        _WIN32_WINNT=0x0A00
        NOMINMAX
    )
endif()

if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(airtraveldb PRIVATE Threads::Threads)
endif()

file(COPY ${CMAKE_SOURCE_DIR}/static DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/include/Models.hpp DESTINATION ${CMAKE_BINARY_DIR}/include)
file(COPY ${CMAKE_SOURCE_DIR}/include/DataManager.hpp DESTINATION ${CMAKE_BINARY_DIR}/include)
file(COPY ${CMAKE_SOURCE_DIR}/src/main.cpp DESTINATION ${CMAKE_BINARY_DIR}/src)
file(COPY ${CMAKE_SOURCE_DIR}/src/DataManager.cpp DESTINATION ${CMAKE_BINARY_DIR}/src)
