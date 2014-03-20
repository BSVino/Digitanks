cmake_minimum_required (VERSION 2.8)

include (${CMAKE_CURRENT_SOURCE_DIR}/cmake/utils.cmake)

project (lw)

if (MSVC)
	set (CMAKE_EXE_LINKER_FLAGS_DEBUG				"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL /SAFESEH:NO" CACHE STRING "" FORCE)
	set (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL			"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE				"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO		"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_DEBUG			"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL /SAFESEH:NO" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL		"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_RELEASE			"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO	"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_DEBUG			"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL /SAFESEH:NO" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL		"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_RELEASE			"/ENTRY:mainCRTStartup /INCREMENTAL:NO" CACHE STRING "" FORCE)
	set (CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO	"/ENTRY:mainCRTStartup /DEBUG /INCREMENTAL:NO" CACHE STRING "" FORCE)
endif ()

option(BUILD_ENGINE "Build Tinker's game engine and whatever games I happen to be working on" ON)
option(WITH_NETWORKING "Build with networking enabled." ON)
option(WITH_EASTL "Use the super-fast (even in debug mode) EASTL STL library." ON)
option(WITH_ASSIMP "Use the unfortunately named AssImp library to load more formats." ON)

if (WITH_EASTL)
	add_definitions(-DWITH_EASTL)
endif (WITH_EASTL)

if (WITH_ASSIMP)
	add_definitions(-DWITH_ASSIMP)
endif ()

if (NOT WITH_NETWORKING)
	add_definitions(-DTINKER_NO_NETWORKING)
endif (NOT WITH_NETWORKING)

if (BUILD_ENGINE)
	add_definitions(-DTINKER_BUILD_ENGINE)
endif ()

if (CMAKE_COMPILER_IS_GNUCC)
	add_definitions(-std=c++0x)
endif (CMAKE_COMPILER_IS_GNUCC)

if (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-D_CRT_NON_CONFORMING_SWPRINTFS)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	add_definitions(-DEA_CHAR16_NATIVE=1)
	add_definitions(-D_UNICODE)
	add_definitions(-DFREEGLUT_STATIC)
	add_definitions(-DFTGL_LIBRARY_STATIC)
endif (WIN32)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions(-DFCOLLADA_DLL)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-D_DEBUG)
endif (CMAKE_BUILD_TYPE MATCHES Debug)

add_subdirectory (common)

add_subdirectory (modelconverter)
add_subdirectory (raytracer)
add_subdirectory (toys)

add_subdirectory (tinker)

if (BUILD_ENGINE)
	add_subdirectory (tengine)
	add_subdirectory (tools)

	add_subdirectory (digitanks)
endif (BUILD_ENGINE)

add_subdirectory (tests)
