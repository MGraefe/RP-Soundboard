cmake_minimum_required(VERSION 3.5.1)

project(rp_soundboard)

include(files.cmake)

# Get prebuilt ffmpeg folders for windows builds
set(ffmpegIncHint "ffmpeg/include")

# figure out platform defaults for certain things
if (WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(defFFmpegLibHint "ffmpeg/lib_win_x64")
		set(defLibSuffix "_win64.dll")
	else()
		set(defFFmpegLibHint "ffmpeg/lib_win_x86")
		set(defLibSuffix "_win32.dll")
	endif()
	set(defPluginDir "%appdata%/TS3Client/plugins")
elseif(APPLE) # untested thanks to MacOS being a pain in the ass to set up in VM
	# mac has no 32 bit variant
	set(defFFmpegLibHint "ffmpeg/lib_mac_x64")
	set(defLibSuffix "_mac.so")
	set(defPluginDir "~/.ts3client/plugins")
else()
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(defFFmpegLibHint "ffmpeg/lib_lin_x64")
		set(defLibSuffix "_linux_amd64.so")
	else()
		set(defFFmpegLibHint "ffmpeg/lib_lin_x86")
		set(defLibSuffix "_linux_x86.so")
	endif()
	set(defPluginDir "~/.ts3client/plugins")
endif()


set(ffmpegLibHint "${CMAKE_CURRENT_SOURCE_DIR}/${defFFmpegLibHint}" CACHE PATH "directory with static ffmpeg libs")
set(libSuffix ${defLibSuffix} CACHE STRING "output library suffix")
set(pluginDir ${defPluginDir} CACHE PATH "TS3 plugin directory (for file copy)")

find_path(ffmpegIncludeDir libavcodec/avcodec.h PATHS ${ffmpegIncHint} NO_DEFAULT_PATH)
find_library(avcodec avcodec PATHS ${ffmpegLibHint} NO_DEFAULT_PATH)
find_library(avformat avformat PATHS ${ffmpegLibHint} NO_DEFAULT_PATH)
find_library(avutil avutil PATHS ${ffmpegLibHint} NO_DEFAULT_PATH)
find_library(swresample swresample PATHS ${ffmpegLibHint} NO_DEFAULT_PATH)

find_package(Qt5 COMPONENTS Core Widgets Gui Network REQUIRED)

# Turn on Qt compilation toolchain things
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn fPIC on
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# version advance script (generates version.h file)
add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/version/version.h"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/version"
	COMMAND python version.py -inc
	DEPENDS ${sources}
)
# suppress warning for disabled autogen on generated files
set_property(SOURCE "src/version/version.h" PROPERTY SKIP_AUTOGEN ON)

# actual library definition
add_library(rp_soundboard SHARED ${sources} "src/version/version.h")
target_link_libraries(rp_soundboard Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Network)
if (WIN32)
	# only link this way on Windows. Linux requires VERY special linker commands, see below
	target_link_libraries(rp_soundboard ${avcodec} ${avformat} ${avutil} ${swresample})
endif()

target_include_directories(rp_soundboard PUBLIC "pluginsdk/include" ${ffmpegIncludeDir})
set_target_properties(rp_soundboard PROPERTIES
	SUFFIX ${libSuffix}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG debug
	RUNTIME_OUTPUT_DIRECTORY_RELEASE release
)

# Special platform dependent compile options
if (MSVC)
	target_sources(rp_soundboard PRIVATE "src/windows/resource.h" "src/windows/Resource.rc")
	target_link_libraries(rp_soundboard wsock32 ws2_32 secur32) # some windows stuff
	target_compile_options(rp_soundboard PRIVATE /MP) # multiprocessor compiling
else()
	if(APPLE)
		target_compile_definitions(rp_soundboard PRIVATE "MACOS")
	else()
		target_compile_definitions(rp_soundboard PRIVATE "LINUX")
	endif()
	# Compile options that are required to NOT get the "recompile with -fPIC"
	# error when linking the ffmpeg libs. Took me many hours to find this out...
	set_target_properties(rp_soundboard PROPERTIES LINK_FLAGS
		"-Wl,-Bsymbolic -Wl,--whole-archive \
		${avcodec} ${avformat} ${avutil} ${swresample} \
		-Wl,--no-whole-archive"
	)
endif()


# copy to teamspeak dir command stuff
set(COPY_DLL_TO_TEAMSPEAK_DIR TRUE CACHE BOOL "Copy the soundboard DLL to teamspeaks directory")
if (COPY_DLL_TO_TEAMSPEAK_DIR)
	add_custom_command(TARGET rp_soundboard POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rp_soundboard> ${pluginDir}
	)
endif()
