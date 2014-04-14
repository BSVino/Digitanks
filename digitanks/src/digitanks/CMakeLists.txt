if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glfw-2.7.2/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glfw-2.7.2/lib/x11)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tinker
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/digitanks/src/digitanks/
)

include_directories(${CMAKE_SOURCE_DIR}/../ext-deps/SDL2-2.0.2/include)

set (DIGITANKS_SOURCES
	main.cpp

	bot.cpp
	bot_campaign.cpp
	digitanksentity.cpp
	digitanksgame.cpp
	digitankslevel.cpp
	digitanksplayer.cpp
	dt_camera.cpp
	dt_common.cpp
	dt_lobbylistener.cpp
	dt_renderer.cpp
	dt_systems.cpp
	instructor_entity.cpp
	menumarcher.cpp
	powerup.cpp
	selectable.cpp
	supplyline.cpp
	terrain.cpp
	updates.cpp
	wreckage.cpp
	dissolver.cpp
)

set (DIGITANKS_CAMPAIGN_SOURCES
	campaign/campaigndata.cpp
	campaign/campaignentity.cpp
	campaign/campaigninfo.cpp
	campaign/userfile.cpp
)

set (DIGITANKS_STRUCTURES_SOURCES
	structures/autoturret.cpp
	structures/buffer.cpp
	structures/collector.cpp
	structures/cpu.cpp
	structures/loader.cpp
	structures/props.cpp
	structures/resource.cpp
	structures/structure.cpp
)

set (DIGITANKS_UNITS_SOURCES
	units/artillery.cpp
	units/barbarians.cpp
	units/digitank.cpp
	units/maintank.cpp
	units/mechinf.cpp
	units/mobilecpu.cpp
	units/scout.cpp
	units/standardtank.cpp
)

set (DIGITANKS_WEAPONS_SOURCES
	weapons/baseweapon.cpp
	weapons/cameraguided.cpp
	weapons/laser.cpp
	weapons/missiledefense.cpp
	weapons/projectile.cpp
	weapons/specialshells.cpp
	weapons/turretmissile.cpp
)

set (DIGITANKS_UI_SOURCES
	ui/digitankswindow.cpp
	ui/hud.cpp
	ui/lobbyui.cpp
	ui/menu.cpp
	ui/scenetree.cpp
	ui/ui.cpp
	ui/updatespanel.cpp
	ui/weaponpanel.cpp
)

if (WIN32)
	set (DIGITANKS_SOURCES ${DIGITANKS_SOURCES}
		digitanks.rc
	)
endif (WIN32)

source_group(Campaign FILES ${DIGITANKS_CAMPAIGN_SOURCES})
source_group(Structures FILES ${DIGITANKS_STRUCTURES_SOURCES})
source_group(Units FILES ${DIGITANKS_UNITS_SOURCES})
source_group(Weapons FILES ${DIGITANKS_WEAPONS_SOURCES})
source_group(UI FILES ${DIGITANKS_UI_SOURCES})

set (DIGITANKS_EXECUTABLE digitanks)

add_executable (${DIGITANKS_EXECUTABLE} WIN32 ${DIGITANKS_SOURCES} ${DIGITANKS_CAMPAIGN_SOURCES} ${DIGITANKS_STRUCTURES_SOURCES} ${DIGITANKS_UNITS_SOURCES} ${DIGITANKS_WEAPONS_SOURCES} ${DIGITANKS_UI_SOURCES})
target_link_libraries (${DIGITANKS_EXECUTABLE} tinker tengine)

install(TARGETS ${DIGITANKS_EXECUTABLE} RUNTIME DESTINATION .)
