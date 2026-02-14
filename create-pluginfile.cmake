
execute_process(
    COMMAND git describe --tags
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE gitVersionRaw
)
string(STRIP "${gitVersionRaw}" gitVersion)

set(outputFile "${RPSB_PLUGINFILE_OUTPUT_DIR}/rp_soundboard_${gitVersion}.ts3_plugin")

message("Creating final plugin package in ${RPSB_PLUGINFILE_OUTPUT_DIR}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar "cfv" "${outputFile}" --format=zip .
    WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
)
