set "BUILD_TEMP_DIR=..\solutions\%2"

@call cmake_configure.bat CMake %1 %BUILD_TEMP_DIR% %3 "" "-DCMAKE_CONFIGURATION_TYPES:STRING='%4'" "-DCMAKE_BUILD_TYPE:STRING='%4'" "-DMENGINE_LIB_DIR:STRING='%5'" %6 %7
