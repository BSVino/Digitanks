include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${PROJECT_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/vsprojects/include
	${PROJECT_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/src
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tinker
	${CMAKE_SOURCE_DIR}/
)

add_library (toys
	toy.cpp
	toy_util.cpp
	toy.pb.cc
)
