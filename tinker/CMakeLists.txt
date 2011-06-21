if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(GTK2 REQUIRED)
	include_directories(${GTK2_INCLUDE_DIRS})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	../
	../common
	../common/math
)

set (TINKER_SOURCES
	application.cpp
	console.cpp
	cvar.cpp
	portals/portal_stubs.cpp
	register.cpp
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glew/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glfw/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/include)

	set (TINKER_SOURCES ${TINKER_SOURCES}
		${CMAKE_SOURCE_DIR}/ext-deps/glew/src/glew.c
	)
endif (WIN32)

add_library(tinker ${TINKER_SOURCES})
