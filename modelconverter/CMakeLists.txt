if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(LibXml2 REQUIRED)
	include_directories(${LIBXML2_INCLUDE_DIR})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${PROJECT_SOURCE_DIR}/../ext-deps/fcollada/FCollada
	${PROJECT_SOURCE_DIR}/../ext-deps/fcollada/FCollada/LibXML/include
	${PROJECT_SOURCE_DIR}/../ext-deps/fcolladaCE/FCollada
	${PROJECT_SOURCE_DIR}/../ext-deps/assimp/include
	../common
	../common/math
)

set (MODELCONVERTER_SOURCES_FORMATS
	formats/collada.cpp
	formats/obj.cpp
	formats/silo.cpp
	formats/smd.cpp
	formats/assimp.cpp
)


source_group(Formats FILES ${MODELCONVERTER_SOURCES_FORMATS})

add_library (modelconverter
	convmesh.cpp
	modelconverter.cpp
	${MODELCONVERTER_SOURCES_FORMATS}
)
