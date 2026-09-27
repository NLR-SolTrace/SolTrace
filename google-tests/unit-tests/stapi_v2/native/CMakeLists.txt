# mirroring corestrace/stapi_v2/CMakeLists.txt but forcing only native runner to be built and linked in for testing purposes
project(stapi_v2_native)

include_directories(
    .
    ../across_builds
    ${SAMPLE_DIR}
    ${TEST_INPUT_DIR}
    ../../../../coretrace/simulation_data
    ../../../../coretrace/simulation_results
    ../../../../coretrace/simulation_runner/native_runner
)

set(STAPI_V2_NATIVE_SRC
    ../../../../coretrace/stapi_v2/stapi_v2.h
    ../across_builds/across_builds.cpp
	data_tests.cpp
    json_io.cpp
	results_tests.cpp
	runner_tests.cpp
)

add_executable(stapi_v2_native ${STAPI_V2_NATIVE_SRC})

############
# test dll #
############

set(STAPI_V2_NATIVE_DLL_SRC
	../../../../coretrace/stapi_v2/stapi_v2.cpp
)

# Add header files for IDE visibility
set(STAPI_V2_NATIVE_HEADERS
    ../../../../coretrace/stapi_v2/stapi_v2.h
    ../../../../coretrace/simulation_runner/simulation_runner.hpp
)

# Group header files for IDEs
source_group("Header Files" FILES ${STAPI_V2_NATIVE_HEADERS})

add_library(stapi_v2_native_dll SHARED ${STAPI_V2_NATIVE_DLL_SRC} ${STAPI_V2_NATIVE_HEADERS})
set_target_properties(stapi_v2_native_dll
		PROPERTIES
		DEBUG_POSTFIX "d"
		PREFIX ""
		)

# no build options for optional runners

target_link_libraries(stapi_v2_native_dll PRIVATE
    simdata
    simresult
    native_runner
)

if(UNIX)
    target_link_libraries(stapi_v2_native_dll PRIVATE pthread)
endif()

if (NOT APPLE AND ENABLE_COVERAGE)
    target_link_options(stapi_v2_native_dll PRIVATE --coverage)
    target_link_libraries(stapi_v2_native_dll PRIVATE gcov)
endif()

if(MSVC)
    target_compile_definitions(stapi_v2_native_dll PRIVATE STAPI_V2_EXPORTS _STAPI_V2_DLL_)
endif()

install(TARGETS stapi_v2_native_dll
    EXPORT SolTraceTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${STAPI_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SolTrace/stapi_v2_native_dll
)

######################################
# linking/installing test executable #
######################################

target_link_libraries(stapi_v2_native PRIVATE
    simdata
    simresult
    native_runner
    stapi_v2_native_dll
    testtools
    GTest::gtest_main
)

add_custom_command(TARGET stapi_v2_native POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:stapi_v2_native_dll>            # The source DLL from the 'stapi_v2' target
    $<TARGET_FILE_DIR:stapi_v2_native> # The destination folder
)

if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET stapi_v2_native POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_PDB_FILE:stapi_v2_native_dll>             # The source pdb from the 'stapi_v2' target
        $<TARGET_FILE_DIR:stapi_v2_native> # The destination folder
    )
endif()

if (ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_options(stapi_v2_native PRIVATE --coverage)
    target_link_libraries(stapi_v2_native PRIVATE gcov)
endif()

if(UNIX)
    target_link_libraries(stapi_v2_native PRIVATE pthread)
endif()

add_test(NAME stapi_v2_native COMMAND stapi_v2_native)