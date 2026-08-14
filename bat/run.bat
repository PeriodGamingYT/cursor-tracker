@ECHO OFF
CLS
SET StartPath=%CD%
SET RootPath=%~dp0.\..
CALL "%RootPath%\bat\vs-setup-caller-snippet.bat"
SETLOCAL ENABLEDELAYEDEXPANSION

	REM Default parameters
	SET DefaultDoCompile=1

	REM Flag parameters
	SET DoCompile=!DefaultDoCompile!

	SET OtherArgs=
	SET IsOption=0
	FOR %%x IN (%*) DO (
		IF "%%x" == "compile" (
			SET DoCompile=1
			SET IsOption=1
		)

		IF "%%x" == "no-compile" (
			SET DoCompile=0
			SET IsOption=1
		)

		IF "%%x" == "help" (
			ECHO run[.bat] [compile, no-compile]

			CD "!StartPath!"
			ENDLOCAL
			EXIT /B 0
		)

		IF "!IsOption!" == "0" (
			SET OtherArgs=!OtherArgs! %%x
		)

		SET IsOption=0
	)

	IF "!DoCompile!" == "1" (
		CALL "!RootPath!\bat\build.bat" !OtherArgs!
	)

	IF %ERRORLEVEL% NEQ 0 (
		ECHO build.bat failed, exiting with error...

		CD "!StartPath!"
		ENDLOCAL
		EXIT /B 1
	)

	IF EXIST "!RootPath!\obj\result\cursor-tracker.exe" (
		START "" "!RootPath!\obj\result\cursor-tracker.exe"
	)
CD "!StartPath!"
ENDLOCAL
