include_directories (
	${PROJECT_SOURCE_DIR}/../ext-deps/EASTL/include
	${PROJECT_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/vsprojects/include
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/math
	${CMAKE_SOURCE_DIR}/tengine
)

set (TEST_SOURCES
	geometry.cpp
	eangles.cpp
	matrix.cpp
	main.cpp
	toys.cpp
)

add_executable (tests ${TEST_SOURCES})
target_link_libraries (tests lwbase tinker modelconverter toys)

if (WIN32)
	target_link_libraries(tests debug ${CMAKE_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/vsprojects/Debug/libprotobuf.lib)
	target_link_libraries(tests optimized ${CMAKE_SOURCE_DIR}/../ext-deps/protobuf-2.5.0/vsprojects/Release/libprotobuf.lib)
endif (WIN32)
