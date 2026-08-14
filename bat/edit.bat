@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION
CLS
SET StartPath=%CD%
SET RootPath=%~dp0.\..

	REM Default parameters
	SET DefaultEditor=focus.exe
	SET DefaultIncludeAssets=0
	SET DefaultIncludeDocs=0

	REM Flag parameters
	SET IsEditorUpcoming=0
	SET Editor=!DefaultEditor!
	SET IncludeAssets=!DefaultIncludeAssets!
	SET IncludeDocs=!DefaultIncludeDocs!

	SET IsInvalidCommandLine=0

	FOR %%x IN (%*) DO (
		IF "!IsEditorUpcoming!" == "1" (
			SET Editor=%%x
			SET IsEditorUpcoming=0
		)
		IF "%%x" == "use-editor" ( SET IsEditorUpcoming=1
		)

		IF "%%x" == "include-assets" ( SET IncludeAssets=1
		)
		IF "%%x" == "exclude-assets" ( SET IncludeAssets=0
		)

		IF "%%x" == "include-docs" ( SET IncludeDocs=1
		)
		IF "%%x" == "exclude-docs" ( SET IncludeDocs=0
		)

		IF "%%x" == "help" (
			ECHO edit[.bat] [use-editor editor-exe] [include-assets, exclude-assets] [include-docs, exclude-docs] [help]

			CD "!StartPath!"
			ENDLOCAL
			EXIT /B 0
		)
	)

	IF "!IsEditorUpcoming" == "1" (
		ECHO Editor wasn't provided with editor
		SET IsInvalidCommandLine=1
	)

	IF "!IsInvalidCommandLine!" == "1" (
		ECHO Invalid command line arguments were provided, exiting with error...
		ENDLOCAL
	)


	SET PathsToInclude= "!RootPath!\src" "!RootPath!\include" "!RootPath!\bat"
	IF "!IncludeAssets!" == "1" (
		SET PathsToInclude=!PathsToInclude! "!RootPath!\test-assets"
	)

	IF "!IncludeDocs!" == "1" (
		SET PathsToInclude=!PathsToInclude! "!RootPath!\docs"
	)

	START "" "!Editor!" !PathsToInclude!
CD "!StartPath!"
ENDLOCAL
