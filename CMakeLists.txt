cmake_minimum_required (VERSION 2.8)

set (CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_overrides.cmake)
set (CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_overrides.cmake)

project (lw)

if (CMAKE_COMPILER_IS_GNUCC)
	add_definitions(-std=c++0x)
endif (CMAKE_COMPILER_IS_GNUCC)

if (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-D_CRT_NON_CONFORMING_SWPRINTFS)
	add_definitions(-D_CHAR16T)
	add_definitions(-D_UNICODE)
	add_definitions(-DGLEW_STATIC)
	add_definitions(-DFREEGLUT_STATIC)
	add_definitions(-DFTGL_LIBRARY_STATIC)
	add_definitions(-DTINKER_UNLOCKED)
	add_definitions(-DTINKER_NO_REGISTRATION)
endif (WIN32)

if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-D_DEBUG)
endif (CMAKE_BUILD_TYPE MATCHES Debug)

add_subdirectory (lwbase)

add_subdirectory (raytracer)
add_subdirectory (modelconverter)

add_subdirectory (tinker)

add_subdirectory (smak)

add_subdirectory (digitanks)
