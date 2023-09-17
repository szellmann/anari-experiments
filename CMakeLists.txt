project(anari-experiments)

cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/cmake
)

find_package(anari 0.4.0 REQUIRED)
find_package(GLEW REQUIRED)

set(VOLKIT_DIR ${PROJECT_SOURCE_DIR}/volkit)
set(VSNRAY_DIR ${VOLKIT_DIR}/src/3rdparty/visionaray)
set(IMGUI_DIR ${VSNRAY_DIR}//src/3rdparty/imgui)
# Just reuse the Cmdline version that comes with the submodules
set(CMD_LINE_DIR ${VSNRAY_DIR}/src/3rdparty/CmdLine)

add_subdirectory(${VOLKIT_DIR})

# ANARI scene graph
add_subdirectory(asg)

# that's a _bit_ hacky, but the easiest way to set the
# rpath so that dlopen will search for libanari_library_XXX
# in the device lib's binary dir
SET(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,${CMAKE_CURRENT_BINARY_DIR}/stub_device")

# visionaray device (obsolete, use the anari-visionaray device instead: https://github.com/szellmann/anari--visionaray)
# add_subdirectory(generic_device)
# SET(CMAKE_EXE_LINKER_FLAGS
#     "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,${CMAKE_CURRENT_BINARY_DIR}/generic_device")

add_executable(viewer
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${VOLKIT_DIR}/src/vkt/TransfuncEditor.cpp
    hdri.cpp
    viewer.cpp
)

target_include_directories(viewer
    PRIVATE
    ${PROJECT_SOURCE_DIR} # for asg headers
    ${IMGUI_DIR}
    ${VOLKIT_DIR}/include/c
    ${VOLKIT_DIR}/include/cpp
    ${VOLKIT_DIR}/include/shared
    ${CMAKE_CURRENT_BINARY_DIR}/volkit/config
    ${VSNRAY_DIR}/src
    ${CMD_LINE_DIR}/include
)

target_link_libraries(viewer PUBLIC GLEW::GLEW volkit visionaray_common asg anari::anari)

option(ASG_MACOSX_BUNDLE "Build executables as application bundles on macOS" ON)
if(ASG_MACOSX_BUNDLE)
    set_target_properties(viewer PROPERTIES MACOSX_BUNDLE TRUE)
endif()

