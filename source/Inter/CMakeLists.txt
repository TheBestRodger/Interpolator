cmake_minimum_required(VERSION 3.25)
project(CInterpol)
set(MPROJECT_VERSION 1.0.0)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g")#-g

include_directories(${INCLUDE_PATH}/Inter)

file(GLOB_RECURSE   SOURCE     ${SOURCE_PATH}/Inter/*.c)
file(GLOB_RECURSE   HEADER     ${INCLUDE_PATH}/Inter/*.h)

add_library(CInterpol ${HEADER} ${SOURCE})

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/lib)