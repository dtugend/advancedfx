﻿# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)

project ("advancedfx")

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
	set(CMAKE_BUILD_TYPE_DEBUG 1)
endif()

set(VS_CONFIGURATION $<IF:$<CONFIG:Debug>,Debug,Release>)

#
# Get Microsoft Visual Studio related paths:
#

execute_process(
    COMMAND "$ENV{ProgramFiles\(x86\)}\\Microsoft Visual Studio\\Installer\\vswhere.exe" "-latest" "-version" "[17.0,18.0)" "-property" "installationPath"
    OUTPUT_VARIABLE VS_INSTALLPATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "\\" "/" VS_INSTALLPATH ${VS_INSTALLPATH})

execute_process(
    COMMAND "$ENV{ProgramFiles\(x86\)}\\Microsoft Visual Studio\\Installer\\vswhere.exe" "-latest" "-version" "[17.0,18.0)" "-requires" "Microsoft.Component.MSBuild" "-find" "MSBuild\\**\\**\\Bin\\MSBuild.exe"
    OUTPUT_VARIABLE VS_MSBUILD
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "\\" "/" VS_MSBUILD ${VS_MSBUILD})

#
# Include dependencies:
#

include(FetchContent)
include(ExternalProject)

add_subdirectory("deps/release/zlib")
add_subdirectory("deps/release/protobuf")
add_subdirectory("deps/release/openexr")
add_subdirectory("deps/release/injector_bin")

add_subdirectory("AfxCppCli")
add_subdirectory("ShaderBuilder")
add_subdirectory("AfxHookGoldSrc")
add_subdirectory("AfxHookSource")
add_subdirectory("hlae")


#
# advancedfx
#

add_custom_target(advancedfx ALL
	DEPENDS hlae
)

#
# Install
#

set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
if(${CMAKE_BUILD_TYPE_DEBUG})
	set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
endif()
include(InstallRequiredSystemLibraries)
if(${CMAKE_BUILD_TYPE_DEBUG})
	set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} "${MSVC_CRT_DIR}/vccorlib${v}d.dll" "${MSVC_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.${MSVC_REDIST_NAME}.CRT/vccorlib${v}.dll")
else()
	set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} "${MSVC_CRT_DIR}/vccorlib${v}.dll" )
endif()

install(
    PROGRAMS
        ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}

        "${afxcppcli_BINARY_DIR}/AfxCppCli.dll"
        "${afxhookgoldsrc_BINARY_DIR}/AfxHookGoldSrc.dll"
        "${afxhooksource_BINARY_DIR}/AfxHookSource.dll"
        "${hlae_BINARY_DIR}/HLAE.exe"
        "${hlae_BINARY_DIR}/NGettext.dll"
        "${injector_BINARY_DIR}/injector.exe"

        "${zlib_SOURCE_DIR}/zlib1.dll"

		${ILMBASE_BUILD_BINARIES}
		${OPENEXR_BUILD_BINARIES}

    DESTINATION "bin"
)
install(
    FILES
        "${injector_BINARY_DIR}/AfxHook.dat"
    DESTINATION "bin"
)

install(
    FILES
        "${afxcppcli_BINARY_DIR}/AfxCppCli.pdb"
        "${afxhookgoldsrc_BINARY_DIR}/AfxHookGoldSrc.pdb"
        "${afxhooksource_BINARY_DIR}/AfxHookSource.pdb"
        "${hlae_BINARY_DIR}/HLAE.pdb"
        "${injector_BINARY_DIR}/injector.pdb"
    DESTINATION "pdb"
)

install(
    PROGRAMS
        "${injector_BINARY_DIR}/x64/injector.exe"
    DESTINATION "bin/x64"
)
install(
    FILES
        "${injector_BINARY_DIR}/x64/AfxHook.dat"
    DESTINATION "bin/x64"
)
install(
    FILES
        "${injector_BINARY_DIR}/x64/injector.pdb"
    DESTINATION "pdb/x64"
)

install(
	CODE
	"
	execute_process( WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}\" COMMAND \"${CMAKE_SOURCE_DIR}/copy_resources_release.bat\" \"${CMAKE_INSTALL_PREFIX}/bin\" )
	execute_process( WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}\" COMMAND \"${CMAKE_SOURCE_DIR}/l10n_update_to_source.bat\" \"${CMAKE_INSTALL_PREFIX}/bin\" )
	
	execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\" COMMAND \"${CMAKE_COMMAND}\" -E tar cfv  \"${CMAKE_INSTALL_PREFIX}/hlae.zip\" --format=zip -- . )
	execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/pdb\" COMMAND \"${CMAKE_COMMAND}\" -E tar cfv  \"${CMAKE_INSTALL_PREFIX}/hlae_pdb.zip\" --format=zip -- . )

	execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E make_directory installer)
	execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E make_directory installer/hlae)
	execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E copy_directory bin installer/hlae)
	execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E rm -Rf installer/hlae/ffmpeg)
	execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E rm -Rf installer/hlae/locales)
	execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E rm -f installer/hlae/HLAE.exe)
	execute_process( WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/installer/setup\" COMMAND \"${VS_MSBUILD}\" \"${CMAKE_SOURCE_DIR}/installer/setup/setup.wixproj\" \"-property:Configuration=${VS_CONFIGURATION}\" \"-property:Platform=x86\" \"-property:OutputPath=${CMAKE_INSTALL_PREFIX}/installer/build\")
	execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E copy installer/build/HLAE_Setup.exe ./HLAE_Setup.exe)
	execute_process( WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\" COMMAND \"${CMAKE_COMMAND}\" -E copy installer/build/HLAE_Setup.wixpdb ./HLAE_Setup.wixpdb)
	"
)
