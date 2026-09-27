cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(stapi_v2)
# find_package(pybind11 CONFIG REQUIRED)

include_directories(
    .
    ../simulation_data
    ../simulation_results
    ../simulation_runner/native_runner
)

set(STAPI_SRC
	stapi_v2.cpp
)

# Add header files for IDE visibility
set(STAPI_HEADERS
    stapi_v2.h
    ../simulation_runner/simulation_runner.hpp
)

# Group header files for IDEs
source_group("Header Files" FILES ${STAPI_HEADERS})

add_library(stapi_v2 SHARED ${STAPI_SRC} ${STAPI_HEADERS})
set_target_properties(stapi_v2
		PROPERTIES
		DEBUG_POSTFIX "d"
		PREFIX ""
        POSITION_INDEPENDENT_CODE ON
		)

# build options for optional runners
if(SOLTRACE_BUILD_EMBREE_SUPPORT)
    target_include_directories(stapi_v2 PRIVATE
        ../simulation_runner/embree_runner
    )
    target_compile_definitions(stapi_v2 PRIVATE STAPI_V2_EMBREE_SUPPORT)
    target_link_libraries(stapi_v2 PRIVATE embree_runner)
    message(STATUS "[stapi_v2] - Linked Embree")
endif()

if(SOLTRACE_BUILD_OPTIX_SUPPORT)
    target_include_directories(stapi_v2 PRIVATE
        ../simulation_runner/optix_runner
    )
    target_compile_definitions(stapi_v2 PRIVATE STAPI_V2_OPTIX_SUPPORT)
    target_link_libraries(stapi_v2 PRIVATE optix_runner)
    message(STATUS "[stapi_v2] - Linked Optix")
endif()

target_link_libraries(stapi_v2 PRIVATE
    simdata
    simresult
    native_runner
)
message(STATUS "[stapi_v2] - Added simdata, simresult, and native_runner")

if(UNIX)
    target_link_libraries(stapi_v2 PRIVATE pthread)
endif()

if (NOT APPLE AND ENABLE_COVERAGE)
    target_link_options(stapi_v2 PRIVATE --coverage)
    target_link_libraries(stapi_v2 PRIVATE gcov)
endif()

if(MSVC)
    target_compile_definitions(stapi_v2 PRIVATE STAPI_V2_EXPORTS _STAPI_V2_DLL_)
endif()

# Link Libraries and Options
if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

add_custom_command(
    TARGET stapi_v2
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy $<TARGET_FILE:stapi_v2> ${CMAKE_CURRENT_SOURCE_DIR}/pysoltrace
    )

install(TARGETS stapi_v2
    EXPORT SolTraceTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${STAPI_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SolTrace/stapi_v2
)
message(STATUS "[stapi_v2] - Installed.")
