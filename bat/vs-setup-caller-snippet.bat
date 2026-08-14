CLS
SET StartPath=%CD%
SET RootPath=%~dp0.\..
	WHERE /Q CL
	IF %ERRORLEVEL% NEQ 0 (
		ECHO Visual Studio environment variables weren't detected, loading them now...
		CALL "%RootPath%\bat\vs-setup.bat"

		WHERE /Q CL
		IF %ERRORLEVEL% NEQ 0 (
			ECHO Visual Studio failed to setup, exiting with error...

			CD "%StartPath%"
			EXIT /B 1
		)
	)
CD %StartPath%
