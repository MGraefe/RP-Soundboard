cmake_minimum_required(VERSION 3.5.1)

project(rp_soundboard)

include(files.cmake)

# Get prebuilt ffmpeg folders for windows builds
set(ffmpegLibHint "")
set(ffmpegIncHint "")
set(libSuffix "")
if (MSVC)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(ffmpegLibHint "ffmpeg/lib_win_x64")
		set(libSuffix "_win64.dll")
	else()
		set(ffmpegLibHint "ffmpeg/lib_win_x86")
		set(libSuffix "_win32.dll")
	endif()
	set(ffmpegIncHint "ffmpeg/include")
else()
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(libSuffix "_lin64.dll")
	else()
		set(libSuffix "_lin32.dll")
	endif()
endif()


find_path(ffmpegIncludeDir libavcodec/avcodec.h HINTS ${ffmpegIncHint})
find_library(avcodec avcodec HINTS ${ffmpegLibHint})
find_library(avformat avformat HINTS ${ffmpegLibHint})
find_library(avutil avutil HINTS ${ffmpegLibHint})
find_library(swresample swresample HINTS ${ffmpegLibHint})

find_package(Qt5 COMPONENTS Core Widgets Gui Network REQUIRED)

# Turn on Qt compilation toolchain things
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
target_link_libraries(rp_soundboard
	Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Network
	${avcodec} ${avformat} ${avutil} ${swresample}
)
target_include_directories(rp_soundboard PUBLIC "pluginsdk/include" ${ffmpegIncludeDir})
set_target_properties(rp_soundboard PROPERTIES
	SUFFIX ${libSuffix}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG debug
	RUNTIME_OUTPUT_DIRECTORY_RELEASE release
)

if (WIN32)
	target_sources(rp_soundboard PRIVATE "src/windows/resource.h" "src/windows/Resource.rc")
	target_link_libraries(rp_soundboard wsock32 ws2_32 secur32) # some windows stuff
	target_compile_options(rp_soundboard PRIVATE /MP) # multiprocessor compiling
	
	# copy to teamspeak dir command stuff
	set(COPY_DLL_TO_TEAMSPEAK_DIR TRUE CACHE BOOL "Copy the soundboard DLL to teamspeaks directory")
	if (COPY_DLL_TO_TEAMSPEAK_DIR)
		add_custom_command(TARGET rp_soundboard POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rp_soundboard> "%appdata%/TS3Client/plugins"
		)
	endif()
endif()
