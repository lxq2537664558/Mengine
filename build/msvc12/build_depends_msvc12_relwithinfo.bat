@echo off

set build_dir=%cd%
set cmd_vcvars="c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
if exist %cmd_vcvars% goto vs_vars_found
set cmd_vcvars="c:\Program Files\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
:vs_vars_found
if not exist %cmd_vcvars% goto vs_not_found
set cmd_msbuild=msbuild

call %cmd_vcvars% x86
if errorlevel 1 goto error

@echo Starting dependencies build release with debug info configuration...

@pushd ..
@call cmake_configure CMake "%CD%\..\CMake\Depends_WIN32" "..\dependencies\build_msvc12_relwithdebinfo" "Visual Studio 12 2013" "" "-DCMAKE_CONFIGURATION_TYPES:STRING='RelWithDebInfo'" "-DCMAKE_BUILD_TYPE:STRING='RelWithDebInfo'"
@popd

@pushd ..\..\dependencies\build_msvc12_relwithdebinfo
CMake --build .\ --config RelWithDebInfo
@popd

@echo Done
goto exit

:vs_not_found
echo Visual Studio 2012 not found. Make sure it is installed to standard directory.
:error
if not "%1"=="batch" pause
exit /b 1
:exit
@pause