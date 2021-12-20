# ANARI uses a plugin mechanism and requires the
# implementation library to be called anari_library_XXX,
# where XXX is the implemetation-specific name
project(anari_library_generic)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(anari REQUIRED)
find_package(GLEW REQUIRED)

set(VSNRAY_DIR ${PROJECT_SOURCE_DIR}/../volkit/src/3rdparty/visionaray)
message(${VSNRAY_DIR})

add_library(${PROJECT_NAME}
    SHARED # Plugin, so make sure to build shared object!
    ao.cpp
    array.cpp
    backend.cpp
    camera.cpp
    device.cpp
    frame.cpp
    geometry.cpp
    group.cpp
    instance.cpp
    light.cpp
    logging.cpp
    material.cpp
    matte.cpp
    pathtracer.cpp
    perspectivecamera.cpp
    pointlight.cpp
    renderer.cpp
    resource.cpp
    spatialfield.cpp
    surface.cpp
    structuredregular.cpp
    trianglegeom.cpp
    volume.cpp
    world.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${VSNRAY_DIR}/include
    ${VSNRAY_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PUBLIC GLEW::GLEW visionaray_common anari::anari)


