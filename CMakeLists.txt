#CMAKE MAIN
set(MCMAKE_VERSION 3.21)
set(MPROJECT_VERSION 1.0.0)

cmake_minimum_required(VERSION ${MCMAKE_VERSION})

include(settings.cmake)

project(CInterpol)

add_subdirectory(source/test)
# Separation of project to subprojects 
add_subdirectory(source/Inter)
