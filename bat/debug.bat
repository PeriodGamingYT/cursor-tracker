@ECHO OFF
CLS
SET StartPath=%CD%
SET RootPath=%~dp0.\..
CALL "%RootPath%\bat\vs-setup-caller-snippet.bat"
SETLOCAL ENABLEDELAYEDEXPANSION

	REM Default parameters
	SET DefaultDebugger=remedybg.exe

	REM Flag parameters
	SET NoCompile=0

	SET IsDebuggerUpcoming=0
	SET Debugger=!DefaultDebugger!

	SET IsInvalidCommandLine=0

	SET OtherArgs=
	SET IsOption=0
	FOR %%x IN (%*) DO (
		IF "%%x" == "no-compile" (
			SET NoCompile=1
			SET IsOption=1
		)

		IF "!IsDebuggerUpcoming!" == "1" (
			SET Debugger=%%x
			SET IsDebuggerUpcoming=0
			SET IsOption=1
		)

		IF "%%x" == "use-debugger" (
			SET IsDebuggerUpcoming=1
			SET IsOption=1
		)

		IF "%%x" == "help" (
			ECHO debug[.bat] [no-compile] [use-debugger debugger-exe] [help]

			CD "!StartPath!"
			ENDLOCAL
			EXIT /B 0
		)

		IF "!IsOption!" == "0" (
			SET OtherArgs=!OtherArgs! %%x
		)

		SET IsOption=0
	)

	IF "!IsDebuggerUpcoming!" == "1" (
		ECHO Debugger wasn't provided with use-debugger
		SET IsInvalidCommandLine=1
	)

	IF "!IsInvalidCommandLine!" == "1" (
		ECHO Invalid command line arguments were provided, exiting with error...

		ENDLOCAL
		CD !StartPath!
		EXIT /B 1
	)

	IF "!NoCompile!" == "0" (
		CALL "!RootPath!\bat\build.bat" debug !OtherArgs!
		IF NOT "!ERRORLEVEL!" == "0" (
			ECHO build.bat failed, exiting with error...

			ENDLOCAL
			CD !StartPath!
			EXIT /B 1
		)
	)

	IF EXIST "!RootPath!\obj\result\cursor-tracker.exe" (
		TASKLIST /FI "IMAGENAME eq !Debugger!" 2>NUL | FIND /I "!Debugger!" >NUL
		IF "!ERRORLEVEL!" == "0" ( TASKKILL /F /IM !Debugger! )
		START !Debugger! "!RootPath!\obj\result\cursor-tracker.exe"
	)
CD "!StartPath!"
ENDLOCAL
