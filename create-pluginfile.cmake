
execute_process(
    COMMAND git describe --tags
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE gitVersionRaw
)
string(STRIP "${gitVersionRaw}" gitVersion)

set(PLUGIN_FILENAME "rp_soundboard_${gitVersion}.ts3_plugin")
set(PLUGIN_OUTPUT "${RPSB_PLUGINFILE_OUTPUT_DIR}/${PLUGIN_FILENAME}")

message("Creating final plugin package in ${RPSB_PLUGINFILE_OUTPUT_DIR}")

if(WIN32)
    execute_process(
        COMMAND "C:/Program Files/7-Zip/7z.exe" a "${PLUGIN_OUTPUT}" -tzip -mx=9 -mm=Deflate "*"
        WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
    )
else()
    # Use zip on Linux/macOS
    execute_process(
        COMMAND zip -r -9 "${PLUGIN_OUTPUT}" .
        WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
    )
endif()
