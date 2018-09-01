# Date: 2014-07-18
# Author: Gerard Choinka (gerard.choinka+gpcxx@gmail.com)

if ( CMAKE_CXX_COMPILER MATCHES ".*em\\+\\+" )
    
    message( STATUS "Found CXX is Emscripten, adding extra html targets." )

    add_executable ( artificial_ant.html artificial_ant.cpp )

endif()


add_executable ( artificial_ant artificial_ant.cpp )
add_executable ( artificial_ant_gpm artificial_ant_gpm.cpp )

add_subdirectory ( detail )
