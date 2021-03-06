@echo off

if ["%~1"]==[""] (
  @echo invalid arguments, please select configuration
  goto end
)

set "CONFIGURATION=%1"

@echo Starting make solution %CONFIGURATION% configuration...

@pushd %~dp0..
@call vcvarsall_msvc15.bat
@popd

@pushd %~dp0..
@call make_solution.bat "%CD%\..\CMake\Win32" solution_msvc15\%CONFIGURATION% "Visual Studio 15 2017" %CONFIGURATION% build_msvc15\%CONFIGURATION%
@popd

:end
@echo Done

@pause
