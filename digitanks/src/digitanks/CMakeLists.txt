include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/digitanks/src/digitanks/
)

if(WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glew/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/devil-1.7.8/include)
	include_directories(${CMAKE_SOURCE_DIR}/ext-deps/glfw/include)
endif(WIN32)

set (DIGITANKS_SOURCES
	main.cpp

	bot.cpp
	bot_campaign.cpp
	digitanksentity.cpp
	digitanksgame.cpp
	digitankslevel.cpp
	digitanksteam.cpp
	dt_camera.cpp
	dt_common.cpp
	dt_lobbylistener.cpp
	dt_renderer.cpp
	instructor_entity.cpp
	menumarcher.cpp
	powerup.cpp
	selectable.cpp
	supplyline.cpp
	terrain.cpp
	updates.cpp
	wreckage.cpp

	campaign/campaigndata.cpp
	campaign/campaignentity.cpp
	campaign/campaigninfo.cpp
	campaign/userfile.cpp

	structures/autoturret.cpp
	structures/buffer.cpp
	structures/collector.cpp
	structures/cpu.cpp
	structures/loader.cpp
	structures/props.cpp
	structures/resource.cpp
	structures/structure.cpp

	units/artillery.cpp
	units/barbarians.cpp
	units/digitank.cpp
	units/maintank.cpp
	units/mechinf.cpp
	units/mobilecpu.cpp
	units/scout.cpp
	units/standardtank.cpp

	weapons/baseweapon.cpp
	weapons/cameraguided.cpp
	weapons/laser.cpp
	weapons/missiledefense.cpp
	weapons/projectile.cpp
	weapons/specialshells.cpp
	weapons/turretmissile.cpp

	ui/digitankswindow.cpp
	ui/digitankswindow_input.cpp
	ui/hud.cpp
	ui/instructor.cpp
	ui/lobbyui.cpp
	ui/menu.cpp
	ui/scenetree.cpp
	ui/ui.cpp
	ui/updatespanel.cpp
	ui/weaponpanel.cpp

	${CMAKE_SOURCE_DIR}/tengine/renderer/systems.cpp
)

if (WIN32)
	set (DIGITANKS_SOURCES ${DIGITANKS_SOURCES}
		digitanks.rc
	)
endif (WIN32)

set (DIGITANKS_EXECUTABLE digitanks)
if (DEMO)
	set (DIGITANKS_EXECUTABLE digitanksdemo)
endif (DEMO)

add_executable (${DIGITANKS_EXECUTABLE} WIN32 ${DIGITANKS_SOURCES})
target_link_libraries (${DIGITANKS_EXECUTABLE} tinker tengine)

install(TARGETS ${DIGITANKS_EXECUTABLE} RUNTIME DESTINATION .)
