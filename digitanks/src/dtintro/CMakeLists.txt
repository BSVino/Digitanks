include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/digitanks/src/digitanks/
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glew/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/include)
endif (WIN32)

set (DTINTRO_SOURCES
	main.cpp

	bomb.cpp
	bug.cpp
	digitank.cpp
	general.cpp
	introtank.cpp
	screen.cpp
	script.cpp

	general_window.cpp
	intro_camera.cpp
	intro_game.cpp
	intro_renderer.cpp
	intro_systems.cpp
	intro_window.cpp
)

if (WIN32)
	set (DTINTRO_SOURCES ${DTINTRO_SOURCES}
		intro_win32.cpp
	)
endif (WIN32)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (DTINTRO_SOURCES ${DTINTRO_SOURCES}
		intro_linux.cpp
	)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

add_executable (dtintro WIN32 ${DTINTRO_SOURCES})
target_link_libraries (dtintro tinker tengine)

install(TARGETS dtintro RUNTIME DESTINATION .)
