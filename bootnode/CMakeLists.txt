cmake_minimum_required(VERSION 3.15)
project(AxionBootnode VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Bootnode library
add_library(axion_bootnode STATIC
    bootnode/bootnode_service.cpp
    bootnode/bootnode_service.hpp
)

target_include_directories(axion_bootnode PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Bootnode executable
add_executable(axion-bootnode
    bootnode/main.cpp
)

target_link_libraries(axion-bootnode PRIVATE
    axion_bootnode
    pthread
)

# Compiler flags
if(MSVC)
    target_compile_options(axion-bootnode PRIVATE /W4 /WX)
    target_compile_options(axion_bootnode PRIVATE /W4 /WX)
else()
    target_compile_options(axion-bootnode PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(axion_bootnode PRIVATE -Wall -Wextra -Wpedantic)
    
    if(UNIX AND NOT APPLE)
        target_link_libraries(axion-bootnode PRIVATE pthread)
    endif()
endif()

# Installation
install(TARGETS axion-bootnode
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Summary
message(STATUS "")
message(STATUS "╔════════════════════════════════════════════════════════════════╗")
message(STATUS "║        AXION BOOTNODE BUILD CONFIGURATION                      ║")
message(STATUS "╠════════════════════════════════════════════════════════════════╣")
message(STATUS "║ Project:        ${PROJECT_NAME}")
message(STATUS "║ Version:        ${PROJECT_VERSION}")
message(STATUS "║ C++ Standard:   C++${CMAKE_CXX_STANDARD}")
message(STATUS "║ Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "║ Output:         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "╚════════════════════════════════════════════════════════════════╝")
message(STATUS "")
