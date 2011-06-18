include_directories (
	${PROJECT_SOURCE_DIR}/ext-deps/EASTL/include
	../
	../common
	../common/math
)

add_library(tinker application.cpp console.cpp cvar.cpp portals/portal_stubs.cpp register.cpp)
