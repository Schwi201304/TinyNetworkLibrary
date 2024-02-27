cmake_minimum_required(VERSION 3.20)

project(tiny_network C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADER_FILES "${PROJECT_SOURCE_DIR}/src/*.hpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC 
                            ${PROJECT_SOURCE_DIR}/include
                            ${PROJECT_SOURCE_DIR}/src)


find_package(fmt REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC 
                                        pthread
                                        fmt::fmt)


set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME})

foreach(HEADER_FILE ${HEADER_FILES})
        get_filename_component(HEADER_DIR ${HEADER_FILE} DIRECTORY)
        string(REPLACE "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}" HEADER_DEST ${HEADER_DIR})
        file(COPY ${HEADER_FILE} DESTINATION ${HEADER_DEST})
endforeach()

message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message(STATUS "SOURCE_FILES: ${SOURCE_FILES}")
message(STATUS "HEADER_FILES: ${HEADER_FILES}")
message(STATUS "LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")



add_subdirectory(example)



enable_testing()
find_package(GTest REQUIRED)
add_subdirectory(test)
