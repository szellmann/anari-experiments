cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(AnariVTKVolume)

find_package(VTK COMPONENTS 
  CommonColor
  CommonCore
  CommonDataModel
  FiltersCore
  FiltersGeometry
  FiltersModeling
  InteractionStyle
  IOLegacy #ugrid writer
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
  RenderingAnari
)

find_package(anari 0.4.0 REQUIRED)

if (NOT VTK_FOUND)
  message(FATAL_ERROR "AnariVTKVolume: Unable to find the VTK build folder.")
endif()

# Prevent a "command line is too long" failure in Windows.
set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")
add_executable(AnariVTKVolume MACOSX_BUNDLE main.cpp )
  target_link_libraries(AnariVTKVolume PRIVATE ${VTK_LIBRARIES} anari::anari
)
# vtk_module_autoinit is needed
vtk_module_autoinit(
  TARGETS AnariVTKVolume
  MODULES ${VTK_LIBRARIES}
)
find_package(anari 0.4.0 REQUIRED)
