#CMAKE WorkProject
cmake_minimum_required(VERSION 3.25)
#set(CMAKE_CXX_STANDARD 17)
# Setup project
project(test)
# Set directories for headers search
include_directories(${INCLUDE_PATH})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/test)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
# Making file groups
file(GLOB_RECURSE SOURCE  ${SOURCE_PATH}/test/*.c*)
file(GLOB_RECURSE HEADERS ${INCLUDE_PATH}/test/*.h*)
# Make libarary
add_executable(test ${SOURCE} ${HEADERS})

# Set bin as work directory
#set_property(TARGET test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${BIN_PATH})

# Link of libiniparser
target_link_libraries(test PRIVATE "D:\\Projects\\Interpolators\\C_Interpolator\\bin\\Debug\\lib\\CInterpol.lib")
