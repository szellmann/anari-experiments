# ANARI uses a plugin mechanism and requires the
# implementation library to be called anari_library_XXX,
# where XXX is the implemetation-specific name
project(anari_library_visionaray)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(anari REQUIRED)

add_library(${PROJECT_NAME}
    SHARED # Plugin, so make sure to build shared object!
    device.cpp
    frame.cpp
    logging.cpp
    pathtracer.cpp
    renderer.cpp
    resource.cpp
    spatialfield.cpp
    structuredregular.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC anari::anari)


