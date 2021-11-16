project(anari-scene-graph)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(anari REQUIRED)

set(SOURCES asg.cpp)
add_library(asg ${SOURCES})
target_link_libraries(asg PUBLIC anari::anari)
add_executable(test test.c)
target_link_libraries(test PUBLIC asg anari::anari)


