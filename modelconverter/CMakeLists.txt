include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	../common
	../common/math
)

add_definitions(-DNO_COLLADA)

add_library (modelconverter
	convmesh.cpp
	modelconverter.cpp
	formats/collada.cpp
	formats/obj.cpp
	formats/silo.cpp
	formats/smd.cpp
)
