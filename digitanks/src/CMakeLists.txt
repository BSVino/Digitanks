add_subdirectory(game)
add_subdirectory(dtintro)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(Freetype REQUIRED)
	include_directories(${FREETYPE_INCLUDE_DIRS})

	find_package(DevIL REQUIRED)
	include_directories(${IL_INCLUDE_DIR})

	find_package(SDL_mixer REQUIRED)
	include_directories(${SDLMIXER_INCLUDE_DIR})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	${PROJECT_SOURCE_DIR}/ext-deps/enet-1.3.0/include
	.
	../../
	../../common
	../../common/math
	game
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glew/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glfw/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/pthreads-w32-2-8-0-release)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/ftgl-2.1.3~rc5/src)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/freetype-2.3.11/include)
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

	../../tinker/lobby/lobby_client.cpp
	../../tinker/lobby/lobby_server.cpp
	../../tinker/portals/portal_steam.cpp
	../../tinker/gamewindow.cpp
	../../tinker/profiler.cpp
	../../tinker/chatbox.cpp
)

add_library(tengine SHARED ${TENGINE_SOURCES})
target_link_libraries (tengine tinker modelconverter raytracer)

if (WIN32)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/ext-deps/glfw/support/msvc2010/Debug/GLFW.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/ext-deps/ftgl-2.1.3~rc5/msvc/build/ftgl_static_D.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/ext-deps/freetype-2.3.11/objs/win32/vc2010/freetype2311MT_D.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Debug/DevIL.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Debug/ILU.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/ext-deps/pthreads-w32-2-8-0-release/Debug/pthread.lib)

	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/ext-deps/glfw/support/msvc2010/Release/GLFW.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/ext-deps/ftgl-2.1.3~rc5/msvc/build/ftgl_static.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/ext-deps/freetype-2.3.11/objs/win32/vc2010/freetype2311MT.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Release/DevIL.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/msvc10/Release/ILU.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/ext-deps/pthreads-w32-2-8-0-release/Release/pthread.lib)
	target_link_libraries(tengine optimized dbghelp.lib)

	target_link_libraries(tengine opengl32 glu32 iphlpapi ws2_32)
endif (WIN32)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries (tengine ${SDLMIXER_LIBRARY})
	target_link_libraries (tengine ${CMAKE_SOURCE_DIR}/ext-deps/enet-1.3.0/.libs/libenet.a)
	target_link_libraries (tengine ${IL_LIBRARIES} ${ILU_LIBRARIES})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

