@echo off
if "%DevEnvDir%" == "" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\vsdevcmd\ext\vcvars.bat"
if "%DevEnvDir%" == "" call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
if "%DevEnvDir%" == "" call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
if "%DevEnvDir%" == "" goto novs
echo -----------
echo BUILD START
echo -----------
msbuild /nologo /v:m /p:Configuration=Release;Platform=x64 ProjectSpecialK\ProjectSpecialK.vcxproj
if not errorlevel 0 goto nogood
:good
echo ---------------
echo BUILD COMPLETED
echo ---------------
echo Packing...
rd release /s /q > nul
md release
md release\assets
copy ProjectSpecialK\fmodex64.dll release > nul
copy ProjectSpecialK\x64\Release\ProjectSpecialK.exe release > nul
copy ProjectSpecialK\init.json release > nul
cd ProjectSpecialK\data
7za.exe u -r ..\..\release\assets\pskbase.zip *.* > nul
cd ..\..
pause
exit /b 0
:novs
echo.
echo *** No VS2019, 2015, or even 2010 found. ***
echo If you _do_ have it, why not try to build from the IDE?
echo.
:nogood
echo ------------
echo BUILD FAILED
echo ------------
pause
exit /b 1
