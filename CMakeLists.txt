cmake_minimum_required (VERSION 2.8)
project (lw)


if (CMAKE_COMPILER_IS_GNUCC)
	add_definitions(-std=c++0x)
	include_directories(/usr/include/freetype2)
endif (CMAKE_COMPILER_IS_GNUCC)

#add_definitions(-D_UNICODE)

add_subdirectory (raytracer)
add_subdirectory (modelconverter)

add_subdirectory (tinker)

add_subdirectory (smak)
