project(anari-scene-graph)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(anari REQUIRED)
set(VOLKIT_DIR ${PROJECT_SOURCE_DIR}/../volkit)
set(VOLKIT_FOUND TRUE)

set(SOURCES asg.cpp)
add_library(asg ${SOURCES})
target_link_libraries(asg PUBLIC anari::anari)
if(VOLKIT_FOUND)
    target_include_directories(asg
        PRIVATE
        ${VOLKIT_DIR}/include/c
        ${VOLKIT_DIR}/include/cpp
        ${VOLKIT_DIR}/include/shared
        ${CMAKE_CURRENT_BINARY_DIR}/../volkit/config
    )
    target_link_libraries(asg PUBLIC volkit)
    target_compile_definitions(asg PUBLIC ASG_HAVE_VOLKIT=1)
endif()
add_executable(test test.c)
target_link_libraries(test PUBLIC asg anari::anari)

