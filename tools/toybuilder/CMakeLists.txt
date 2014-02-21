include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
	${CMAKE_SOURCE_DIR}/models
)

set (GEPPETTO_SOURCES
	geppetto.cpp
	inputscript.cpp
	toy_physics.cpp
)

set (TOYBUILDER_SOURCES
	main.cpp
)

add_library(geppetto STATIC ${GEPPETTO_SOURCES})
add_executable (toybuilder ${TOYBUILDER_SOURCES})
target_link_libraries (geppetto lwbase tinker modelconverter toys)
target_link_libraries (toybuilder geppetto)

if (WIN32)
	target_link_libraries(toybuilder debug ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Debug/fcollada-2010.lib)
	target_link_libraries(toybuilder debug ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Debug/farchivexml-2010.lib)
	target_link_libraries(toybuilder debug ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Debug/libxml-2010.lib)
	target_link_libraries(toybuilder debug ${CMAKE_SOURCE_DIR}/../ext-deps/assimp/code/Debug/assimp.lib)
	target_link_libraries(toybuilder debug ${CMAKE_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/vsprojects/Debug/libprotobuf.lib)

	target_link_libraries(toybuilder optimized ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Release/fcollada-2010.lib)
	target_link_libraries(toybuilder optimized ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Release/farchivexml-2010.lib)
	target_link_libraries(toybuilder optimized ${CMAKE_SOURCE_DIR}/../ext-deps/fcollada/fcollada-2010/Release/libxml-2010.lib)
	target_link_libraries(toybuilder optimized ${CMAKE_SOURCE_DIR}/../ext-deps/assimp/code/RelWithDebInfo/assimp.lib)
	target_link_libraries(toybuilder optimized ${CMAKE_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/vsprojects/Release/libprotobuf.lib)
endif (WIN32)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(toybuilder ${FREETYPE_LIBRARIES} ${LIBXML2_LIBRARIES})
	target_link_libraries(toybuilder ${CMAKE_SOURCE_DIR}/../ext-deps/fcolladaCE/FCollada/libfcollada.so)
	target_link_libraries(toybuilder ${CMAKE_SOURCE_DIR}/../ext-deps/fcolladaCE/FColladaPlugins/FArchiveXML/libfarchivexml.so)
	target_link_libraries(toybuilder ${CMAKE_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/src/.libs/libprotobuf.a)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

