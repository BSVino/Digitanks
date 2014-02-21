include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	../
	../common
	../common/math
)

if (WIN32)
	include_directories (
		${PROJECT_SOURCE_DIR}/ext-deps/pthreads-w32-2-8-0-release
	)
endif (WIN32)

set (LWBASE_SOURCES
	../ext-deps/EASTL/src/allocator.cpp
	../ext-deps/EASTL/src/assert.cpp
	../ext-deps/EASTL/src/fixed_pool.cpp
	../ext-deps/EASTL/src/hashtable.cpp
	../ext-deps/EASTL/src/red_black_tree.cpp
	../ext-deps/EASTL/src/string.cpp

	../common/configfile.cpp
	../common/eastl.cpp
	../common/mempool.cpp
	../common/mtrand.cpp
	../common/parallelize.cpp
	../common/sockets/sockets.cpp
	../common/math/matrix.cpp
	../common/math/quaternion.cpp
	../common/math/frustum.cpp

	../datamanager/data.cpp
	../datamanager/dataserializer.cpp
)

if (WIN32)
	set (LWBASE_SOURCES ${LWBASE_SOURCES}
		../common/platform_win32.cpp
	)
endif (WIN32)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (LWBASE_SOURCES ${LWBASE_SOURCES}
		../common/platform_linux.cpp
	)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

add_library(lwbase ${LWBASE_SOURCES})

if (WIN32)
	target_link_libraries(lwbase iphlpapi ws2_32 dbghelp)
endif (WIN32)
