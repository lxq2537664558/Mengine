CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(Depends_Xcode_SDL)

INCLUDE(../macro_template.cmake)
INCLUDE(../xcode_template.cmake)

SET_MENGINE_ENVIRONMENT(PYTHON MACOS OPENGL X64 SDL XCODE)

ADD_PLUGIN(MENGINE_PLUGIN_SPINE OFF OFF "MENGINE_PLUGIN_SPINE")
ADD_PLUGIN(MENGINE_PLUGIN_TTF ON OFF "MENGINE_PLUGIN_TTF")
ADD_PLUGIN(MENGINE_PLUGIN_CURL ON OFF "MENGINE_PLUGIN_CURL")
ADD_PLUGIN(MENGINE_PLUGIN_STEAM ON OFF "MENGINE_PLUGIN_STEAM")

SET(MENGINE_REPOSITORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)
SET(THIRDPARTY_DIR ${MENGINE_REPOSITORY}/dependencies)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

SET(CMAKE_TEMP_DIR ${MENGINE_REPOSITORY}/CMakeTemp/${PROJECT_NAME}/${CMAKE_GENERATOR}/${CMAKE_BUILD_TYPE})

ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies ${CMAKE_TEMP_DIR})

