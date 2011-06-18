cmake_minimum_required (VERSION 2.8)
project (lw)


if (CMAKE_COMPILER_IS_GNUCC)
	add_definitions(-fshort-wchar)
endif (CMAKE_COMPILER_IS_GNUCC)

add_subdirectory (raytracer)
add_subdirectory (modelconverter)

add_subdirectory (tinker)
