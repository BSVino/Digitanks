if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(DevIL REQUIRED)
	include_directories(${IL_INCLUDE_DIR})

	find_package(SDL_mixer REQUIRED)
	include_directories(${SDLMIXER_INCLUDE_DIR})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	${PROJECT_SOURCE_DIR}/ext-deps/enet-1.3.0/include
	${PROJECT_SOURCE_DIR}/ext-deps/enet-1.3.3/include
	${CMAKE_SOURCE_DIR}/
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glew/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glfw/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/pthreads-w32-2-8-0-release)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/SDL-1.2.14/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/SDL_mixer-1.2.11/include)
endif (WIN32)

set (TENGINE_SOURCES
	game/baseentity.cpp
	game/camera.cpp
	game/game.cpp
	game/gameserver.cpp
	game/level.cpp
	game/networkedeffect.cpp
	game/team.cpp
	game/entities/counter.cpp

	models/models.cpp
	models/texturelibrary.cpp
	models/texturesheet.cpp

	network/commands.cpp
	network/network.cpp
	network/replication.cpp

	renderer/dissolver.cpp
	renderer/particles.cpp
	renderer/renderer.cpp

	shaders/blur.cpp
	shaders/explosion.cpp
	shaders/filters.cpp
	shaders/model.cpp
	shaders/prop.cpp
	shaders/shaders.cpp
	shaders/terrain.cpp

	sound/sound.cpp

	../tinker/lobby/lobby_client.cpp
	../tinker/lobby/lobby_server.cpp
	../tinker/portals/portal_steam.cpp
	../tinker/gamewindow.cpp
	../tinker/profiler.cpp
	../tinker/chatbox.cpp
)

add_library(tengine STATIC ${TENGINE_SOURCES})
target_link_libraries (tengine tinker modelconverter raytracer)

if (WIN32)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Debug/DevIL.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Debug/ILU.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/ext-deps/pthreads-w32-2-8-0-release/Debug/pthread.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/ext-deps/enet-1.3.0/Debug/enet.lib)

	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Release/DevIL.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Release/ILU.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/ext-deps/pthreads-w32-2-8-0-release/Release/pthread.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/ext-deps/enet-1.3.0/Release/enet.lib)

	target_link_libraries(tengine iphlpapi ws2_32 Winmm)

	target_link_libraries(tengine
		${CMAKE_SOURCE_DIR}/ext-deps/SDL-1.2.14/lib/sdl.lib
		${CMAKE_SOURCE_DIR}/ext-deps/SDL-1.2.14/lib/sdlmain.lib
		${CMAKE_SOURCE_DIR}/ext-deps/SDL_mixer-1.2.11/lib/sdl_mixer.lib
	)
endif (WIN32)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries (tengine ${CMAKE_SOURCE_DIR}/ext-deps/enet-1.3.0/.libs/libenet.a)
	target_link_libraries (tengine ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/lib/.libs/libIL.a)
	target_link_libraries (tengine ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/lib/.libs/libILU.a)
	target_link_libraries (tengine /usr/lib/libpng.a)
	target_link_libraries (tengine /usr/lib/libz.a)
	target_link_libraries (tengine SDL SDL_mixer)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

