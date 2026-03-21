cmake_minimum_required(VERSION 3.18)
project(InZoiTOOL_GUI)

# UE4SS CppMod build
# Set UE4SS_PATH to your UE4SS source/SDK directory
# Example: cmake -DUE4SS_PATH=C:/path/to/RE-UE4SS ..

if(NOT DEFINED UE4SS_PATH)
    message(FATAL_ERROR "Set UE4SS_PATH to your UE4SS source directory")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(main SHARED main.cpp)

target_include_directories(main PRIVATE
    ${UE4SS_PATH}/include
    ${UE4SS_PATH}/vendor/imgui
)

target_link_directories(main PRIVATE
    ${UE4SS_PATH}/lib
)

target_link_libraries(main PRIVATE
    UE4SS
)

# Output directly to dlls/ folder
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
