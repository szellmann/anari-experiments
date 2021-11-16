project(anari-experiments)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(anari REQUIRED)
find_package(GLEW REQUIRED)

set(VOLKIT_DIR ${PROJECT_SOURCE_DIR}/volkit)
set(VSNRAY_DIR ${VOLKIT_DIR}/src/3rdparty/visionaray)
# Just reuse the imgui version that comes with the submodules
set(IMGUI_DIR ${VSNRAY_DIR}/src/3rdparty/imgui)
# Cmdline, too
set(CMD_LINE_DIR ${VSNRAY_DIR}/src/3rdparty/CmdLine)

add_subdirectory(${VOLKIT_DIR})

# ANARI scene graph
add_subdirectory(asg)

add_subdirectory(stub_device)
# that's a _bit_ hacky, but the easiest way to set the
# rpath so that dlopen will search for libanari_library_XXX
# in the device lib's binary dir
SET(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,${CMAKE_CURRENT_BINARY_DIR}/stub_device")

# visionaray device
add_subdirectory(generic_device)
SET(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,${CMAKE_CURRENT_BINARY_DIR}/generic_device")

add_executable(viewer
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${VOLKIT_DIR}/src/vkt/TransfuncEditor.cpp
    viewer.cpp
)

target_include_directories(viewer
    PRIVATE
    ${IMGUI_DIR}
    ${VOLKIT_DIR}/include/c
    ${VOLKIT_DIR}/include/cpp
    ${VOLKIT_DIR}/include/shared
    ${CMAKE_CURRENT_BINARY_DIR}/volkit/config
    ${VSNRAY_DIR}/src
    ${CMD_LINE_DIR}/include
)

target_link_libraries(viewer PUBLIC GLEW::GLEW volkit visionaray_common asg anari::anari)


