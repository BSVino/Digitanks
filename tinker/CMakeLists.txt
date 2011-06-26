if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(Freetype REQUIRED)
	include_directories(${FREETYPE_INCLUDE_DIRS})
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

	../glgui/glgui.cpp

	../license/license.cpp
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glew/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glfw/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/include)

	set (TINKER_SOURCES ${TINKER_SOURCES}
		${CMAKE_SOURCE_DIR}/ext-deps/glew/src/glew.c
	)
endif (WIN32)

add_library(tinker SHARED ${TINKER_SOURCES})
target_link_libraries(tinker lwbase)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries (tinker glfw GLEW ftgl)
	target_link_libraries (tinker ${FREETYPE_LIBRARY})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
