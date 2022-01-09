project(anari-scene-graph)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(anari REQUIRED)
find_package(assimp)
#find_package(pbrtParser)
set(PBRT_PARSER_DIR ${PROJECT_SOURCE_DIR}/../volkit/src/3rdparty/visionaray/src/3rdparty/pbrt-parser)
set(PBRT_PARSER_FOUND TRUE)
#find_package(volkit)
set(VOLKIT_DIR ${PROJECT_SOURCE_DIR}/../volkit)
set(VOLKIT_FOUND TRUE)

set(SOURCES asg.cpp)
add_library(asg ${SOURCES})
target_link_libraries(asg PUBLIC anari::anari)
if (assimp_FOUND)
    target_link_libraries(asg PUBLIC assimp::assimp)
    target_compile_definitions(asg PUBLIC ASG_HAVE_ASSIMP=1) # TODO: config file!
endif()
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
if(PBRT_PARSER_FOUND)
    target_include_directories(asg
        PRIVATE
        ${PBRT_PARSER_DIR}/pbrtParser/include
    )
    target_link_libraries(asg PUBLIC pbrtParser_shared)
    target_compile_definitions(asg PUBLIC ASG_HAVE_PBRT_PARSER=1)
endif()
add_executable(test test.c)
target_link_libraries(test PUBLIC asg anari::anari)


