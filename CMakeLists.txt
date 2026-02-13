cmake_minimum_required(VERSION 3.12.0)

project(rp_soundboard)

include(files.cmake)

if (WIN32)
	# Get prebuilt ffmpeg folders for windows builds
	
else()
	# Use the ffmpeg folder inside official sources
	
endif()

# figure out platform defaults for certain things
if (WIN32)
	set(ffmpegIncHint "ffmpeg/include")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(defFFmpegLibHint "ffmpeg/lib_win_x64")
		set(defLibSuffix "_win64.dll")
	else()
		set(defFFmpegLibHint "ffmpeg/lib_win_x86")
		set(defLibSuffix "_win32.dll")
	endif()
	set(defPluginDir "%appdata%/TS3Client")
elseif(APPLE) # untested thanks to MacOS being a pain in the ass to set up in VM
	# mac has no 32 bit variant
	set(defFFmpegLibHint "ffmpeg/lib_mac_x64")
	set(defLibSuffix "_mac.so")
	set(defPluginDir "~/.ts3client")
else()
	set(ffmpegIncHint "ffmpeg/ffmpeg/x64/include")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(defFFmpegLibHint "ffmpeg/ffmpeg/x64/lib")
		set(defLibSuffix "_linux_amd64.so")
	else()
		set(defFFmpegLibHint "ffmpeg/ffmpeg/x86/lib")
		set(defLibSuffix "_linux_x86.so")
	endif()
	set(defPluginDir "~/.ts3client")
endif()


set(ffmpegLibHint "${CMAKE_CURRENT_SOURCE_DIR}/${defFFmpegLibHint}" CACHE PATH "directory with static ffmpeg libs")
set(libSuffix ${defLibSuffix} CACHE STRING "output library suffix")


if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${defPluginDir}" CACHE PATH "..." FORCE)
endif()

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
add_custom_target(autogenFiles ALL
	BYPRODUCTS "src/version/version.h" "deploy/package.ini"
	COMMENT "running autogen file script"
	COMMAND python version.py
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
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

install(TARGETS rp_soundboard
	LIBRARY DESTINATION "plugins"
	RUNTIME DESTINATION "plugins"
)
install(DIRECTORY "deploy/" DESTINATION ".")

set(RPSB_MAKE_PLUGIN_FILE OFF CACHE BOOL "Create the ts3_plugin file using 7zip")

if (${RPSB_MAKE_PLUGIN_FILE})
	set(RPSB_PLUGINFILE_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/release" CACHE PATH "Output directory for the final plugin file")

	install(CODE "set(RPSB_PLUGINFILE_OUTPUT_DIR \"${RPSB_PLUGINFILE_OUTPUT_DIR}\")") # forward var
	install(SCRIPT "create-pluginfile.cmake")
endif()
