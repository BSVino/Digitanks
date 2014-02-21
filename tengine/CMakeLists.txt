if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(SDL_mixer REQUIRED)
	include_directories(${SDLMIXER_INCLUDE_DIR})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${PROJECT_SOURCE_DIR}/../ext-deps/enet-1.3.0/include
	${PROJECT_SOURCE_DIR}/../ext-deps/enet-1.3.3/include
	${PROJECT_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/vsprojects/include
	${PROJECT_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/src
	${PROJECT_SOURCE_DIR}/../ext-deps/bullet-2.81/src
	${CMAKE_SOURCE_DIR}/
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tinker
	${CMAKE_SOURCE_DIR}/tengine
)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/gl3w/include)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/glfw-1.3/include)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/pthreads-w32-2-8-0-release)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/SDL-1.2.14/include)
	include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/SDL_mixer-1.2.11/include)
endif (WIN32)

set (TENGINE_GAME_SOURCES
	game/cameramanager.cpp
	game/gameserver.cpp
	game/level.cpp
	game/networkedeffect.cpp
)

set (TENGINE_GAME_ENTITY_SOURCES
	game/entities/baseentity.cpp
	game/entities/baseentitydata.cpp
	game/entities/beam.cpp
	game/entities/camera.cpp
	game/entities/counter.cpp
	game/entities/character.cpp
	game/entities/charactercamera.cpp
	game/entities/game.cpp
	game/entities/kinematic.cpp
	game/entities/logicgate.cpp
	game/entities/mathgate.cpp
	game/entities/player.cpp
	game/entities/playerstart.cpp
	game/entities/prop.cpp
	game/entities/static.cpp
	game/entities/target.cpp
	game/entities/team.cpp
	game/entities/trigger.cpp
	game/entities/weapon.cpp
)

set (TENGINE_MODELS_SOURCES
	models/models.cpp
	models/loadsource.cpp
)

set (TENGINE_NETWORK_SOURCES
	network/commands.cpp
	network/replication.cpp
)

if (WITH_NETWORKING)
	set (TENGINE_NETWORK_SOURCES ${TENGINE_NETWORK_SOURCES}
		network/network.cpp
	)
else (WITH_NETWORKING)
	set (TENGINE_NETWORK_SOURCES ${TENGINE_NETWORK_SOURCES}
		network/network_loopback.cpp
	)
endif (WITH_NETWORKING)

set (TENGINE_PHYSICS_SOURCES
	physics/bullet_physics.cpp
	physics/character_controller.cpp
	physics/physics_debugdraw.cpp
	physics/trigger_controller.cpp
)

set (TENGINE_RENDERER_SOURCES
	renderer/particles.cpp
	renderer/game_renderer.cpp
	renderer/game_renderingcontext.cpp
	renderer/roperenderer.cpp
)

set (TENGINE_SOUND_SOURCES
	sound/sound.cpp
)

set (TENGINE_UI_SOURCES
	ui/hudviewport.cpp
	ui/gamewindow.cpp
	ui/chatbox.cpp
	ui/instructor.cpp
)

set (TENGINE_LOBBY_SOURCES
	lobby/lobby_client.cpp
	lobby/lobby_server.cpp
)

set (TENGINE_TOOLS_LEVELEDITOR_SOURCES
	tools/leveleditor/entitypropertiespanel.cpp
	tools/leveleditor/editorpanel.cpp
	tools/leveleditor/createentitypanel.cpp
	tools/leveleditor/leveleditor.cpp
)

set (TENGINE_TOOLS_SOURCES
	tools/materialeditor.cpp
	tools/toyeditor.cpp
	tools/toyviewer.cpp
	tools/tool.cpp
	tools/workbench.cpp
)

set (TENGINE_PORTALS_SOURCES
	portals/portal_stubs.cpp
)

set (TENGINE_SOURCES
	${TENGINE_GAME_SOURCES}
	${TENGINE_GAME_ENTITY_SOURCES}
	${TENGINE_MODELS_SOURCES}
	${TENGINE_NETWORK_SOURCES}
	${TENGINE_PHYSICS_SOURCES}
	${TENGINE_RENDERER_SOURCES}
	${TENGINE_SOUND_SOURCES}
	${TENGINE_UI_SOURCES}
	${TENGINE_LOBBY_SOURCES}
	${TENGINE_TOOLS_SOURCES}
	${TENGINE_TOOLS_LEVELEDITOR_SOURCES}
	${TENGINE_PORTALS_SOURCES}
)

source_group(Game FILES ${TENGINE_GAME_SOURCES})
source_group(Game\\Entities FILES ${TENGINE_GAME_ENTITY_SOURCES})
source_group(Models FILES ${TENGINE_MODELS_SOURCES})
source_group(Network FILES ${TENGINE_NETWORK_SOURCES})
source_group(Physics FILES ${TENGINE_PHYSICS_SOURCES})
source_group(Renderer FILES ${TENGINE_RENDERER_SOURCES})
source_group(Sound FILES ${TENGINE_SOUND_SOURCES})
source_group(UI FILES ${TENGINE_UI_SOURCES})
source_group(Lobby FILES ${TENGINE_LOBBY_SOURCES})
source_group(Tools FILES ${TENGINE_TOOLS_SOURCES})
source_group(Tools\\LevelEditor FILES ${TENGINE_TOOLS_LEVELEDITOR_SOURCES})
source_group(Portals FILES ${TENGINE_PORTALS_SOURCES})

#Breaks other stuff.
#add_msvc_precompiled_header("game/entities/baseentity.h" "game/entities/baseentity_pch.cpp" TENGINE_SOURCES)

add_library(tengine STATIC ${TENGINE_SOURCES})
target_link_libraries (tengine tinker modelconverter toys geppetto)

if (WIN32)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/../ext-deps/pthreads-w32-2-8-0-release/Debug/pthread.lib)
	if (WITH_NETWORKING)
		target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/../ext-deps/enet-1.3.0/Debug/enet.lib)
	endif (WITH_NETWORKING)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/../ext-deps/bullet-2.81/lib/Debug/BulletDynamics_Debug.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/../ext-deps/bullet-2.81/lib/Debug/BulletCollision_Debug.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/../ext-deps/bullet-2.81/lib/Debug/LinearMath_Debug.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Debug/fcollada-2010.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Debug/farchivexml-2010.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Debug/libxml-2010.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/../ext-deps/assimp/code/Debug/assimp.lib)
	target_link_libraries(tengine debug ${CMAKE_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/vsprojects/Debug/libprotobuf.lib)

	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/../ext-deps/pthreads-w32-2-8-0-release/Release/pthread.lib)
	if (WITH_NETWORKING)
		target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/../ext-deps/enet-1.3.0/Release/enet.lib)
	endif (WITH_NETWORKING)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/../ext-deps/bullet-2.81/lib/RelWithDebInfo/BulletDynamics_RelWithDebugInfo.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/../ext-deps/bullet-2.81/lib/RelWithDebInfo/BulletCollision_RelWithDebugInfo.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/../ext-deps/bullet-2.81/lib/RelWithDebInfo/LinearMath_RelWithDebugInfo.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Release/fcollada-2010.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Release/farchivexml-2010.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Release/libxml-2010.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/../ext-deps/assimp/code/RelWithDebInfo/assimp.lib)
	target_link_libraries(tengine optimized ${CMAKE_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/vsprojects/Release/libprotobuf.lib)

	target_link_libraries(tengine iphlpapi ws2_32 Winmm)

	target_link_libraries(tengine
		${CMAKE_SOURCE_DIR}/../ext-deps/SDL-1.2.14/lib/sdl.lib
		${CMAKE_SOURCE_DIR}/../ext-deps/SDL-1.2.14/lib/sdlmain.lib
		${CMAKE_SOURCE_DIR}/../ext-deps/SDL_mixer-1.2.11/lib/sdl_mixer.lib
	)
endif (WIN32)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	if (WITH_NETWORKING)
		target_link_libraries (tengine ${CMAKE_SOURCE_DIR}/../ext-deps/enet-1.3.0/.libs/libenet.a)
	endif (WITH_NETWORKING)
	target_link_libraries (tengine /usr/lib/libpng.a)
	target_link_libraries (tengine /usr/lib/libz.a)
	target_link_libraries (tengine SDL SDL_mixer)
	target_link_libraries (tengine ${CMAKE_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/src/.libs/libprotobuf.a)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

