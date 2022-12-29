@ECHO OFF

SET ARG=%1
SET RELEASE=

REM If the first command line argument is set to 'release' define release.
IF "%ARG%"=="release" SET RELEASE=release
REM If the first command line arguement is set to 'help' display the help list.
IF "%ARG%"=="help" (
	@ECHO ON
	echo "PRINTING HELP..."
	@ECHO OFF
)

REM Save project root directory path.
SET PROJECT_ROOT=%cd%

SET EXECUTABLE_NAME=sp
SET SRC=..\src

REM Check if 'DevEnvDir' is defined. If not, call vcvarsall.bat.
REM This is to resolve the path growing too large when calling vcvar64.bat multiple times.
IF NOT DEFINED DevEnvDir (
	CALL vcvars64.bat
)

REM I belive calling vcvarsall.bat will start a new cmd and have us in the location of vcvarsall.bat.
REM So, change directory back to project root.
CD %PROJECT_ROOT%

SET CFLAGS=/std:c11 /Wall /Zi /TC

REM Create bin direct, change to it, build the application, and change directory back.
IF NOT EXIST bin MKDIR bin
PUSHD bin

REM Set relase related flags
IF DEFINED RELEASE (
	SET CFLAGS=/std:c11 /TC /Ot
)

cl %CFLAGS% %SRC%\main.c /Fe: %EXECUTABLE_NAME%.exe


POPD
