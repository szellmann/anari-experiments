set(SOURCES asg.cpp)
add_library(asg ${SOURCES})
add_executable(test test.c)
target_link_libraries(test asg)


