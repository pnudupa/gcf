@ECHO OFF

SET ROOT=%CD%
SET OUTPUT_PATH=%ROOT%\Results\Windows\UnitTests
SET PATH=%ROOT%\..\;%ROOT%\UnitTests;%PATH%

SET EXIT_CODE=0

DEL /S /Q Results\*.* > NUL 2> NUL
RD /S /Q Results
MD %OUTPUT_PATH%

ECHO.
CD UnitTests
FOR %%G IN (tst_*.exe) DO	(CALL:processTest %%G)
CD ..
ECHO.

ECHO All test results are stored in : 
ECHO %OUTPUT_PATH%

EXIT /B %EXIT_CODE%

REM Custom functions
REM -----------------------------------------

REM processTest <TestBinaryName>
:processTest
	%~1 > %OUTPUT_PATH%\%~N1.txt 2> NUL
	IF %ERRORLEVEL%==0 (CALL:markSuccess %ERRORLEVEL% %~1) ELSE (CALL:markFailure %ERRORLEVEL% %~1)
GOTO:EOF

REM markSuccess <ErrorLevel> <TestBinaryName>
:markSuccess
	SET ERROR_CODE=    %~1
	SET ERROR_CODE=%ERROR_CODE:~-4%
	ECHO  [%ERROR_CODE%] - %~2
GOTO:EOF

REM markFailure <ErrorLevel> <TestBinaryName>
:markFailure
	SET ERROR_CODE=    %~1
	SET ERROR_CODE=%ERROR_CODE:~-4%
	ECHO *[%ERROR_CODE%] - %~2
	
	FOR /F "delims=:" %%A IN ('FINDSTR /N /C:"FAIL!" %OUTPUT_PATH%\%~N2.txt') DO (
		@SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
		SET index=0
		SET /A line1=%%A-1
		SET /A line2=%%A
		SET /A line3=%%A+1
		SET /A line4=%%A+2
		FOR /F "delims=" %%L IN ('TYPE %OUTPUT_PATH%\%~N2.txt') DO (
			IF !index!==!line1! ECHO     %%L
			IF !index!==!line2! ECHO     %%L
			IF !index!==!line3! ECHO     %%L
			IF !index!==!line4! ECHO     %%L
			SET /A index+=1
		)
		@ENDLOCAL
	)
	
	SET EXIT_CODE=1
GOTO:EOF
