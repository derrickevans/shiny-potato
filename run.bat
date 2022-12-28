@ECHO OFF

SET EXECUTABLE_NAME=sp

IF NOT EXIST bin (
	@ECHO ON
	ECHO "bin directory does not exist. Nothing to run."
	@ECHO OFF
	EXIT /B
)

.\bin\%EXECUTABLE_NAME%.exe
