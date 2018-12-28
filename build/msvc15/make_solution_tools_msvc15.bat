@echo off

set "CONFIGURATION=%1"

@echo Starting make solution tools %CONFIGURATION% configuration...

@pushd ..
@call vcvarsall_msvc15.bat
@popd

@pushd ..
@call make_solution.bat "%CD%\..\CMake\Tools_Win32" solution_tools_msvc15\%CONFIGURATION% "Visual Studio 15 2017" %CONFIGURATION% build_msvc15\%CONFIGURATION%
@popd

@echo Done

@pause
