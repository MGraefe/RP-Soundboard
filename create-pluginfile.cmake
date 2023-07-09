
execute_process(
    COMMAND git describe --tags
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE gitVersionRaw
)
string(STRIP "${gitVersionRaw}" gitVersion)

message("Running 7zip to create final plugin package in ${RPSB_PLUGINFILE_OUTPUT_DIR}")
execute_process(
    COMMAND "C:/Program Files/7-Zip/7z.exe" a "${RPSB_PLUGINFILE_OUTPUT_DIR}/rp_soundboard_${gitVersion}.ts3_plugin" -tzip -mx=9 -mm=Deflate "*"
    WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
)
