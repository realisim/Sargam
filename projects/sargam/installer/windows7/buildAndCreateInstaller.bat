@echo off

if "%1" == "" goto error
if "%2" == "-i" goto installer

REM run cmake (cmake.exe must be in the path)

REM clean
pushd "c:\Program Files (x86)\MSBuild\12.0\Bin\"
"C:\Program Files (x86)\MSBuild\12.0\Bin\MSBuild" /t:clean /p:configuration=RELEASE e:\code\CmakeRealisim\Realisim.sln

REM build
"C:\Program Files (x86)\MSBuild\12.0\Bin\MSBuild" /t:build /p:configuration=RELEASE e:\code\CmakeRealisim\Realisim.sln
popd

REM windeploy
pushd E:\Qt\Qt5.4.2\5.4\msvc2013_64\bin
set PATH=e:\Qt\Qt5.4.2\5.4\msvc2013_64\bin;%PATH%
windeployqt.exe e:\code\CmakeRealisim\projects\sargam\Release\sargam.exe
popd

:installer
echo %1
pushd "E:\Program Files (x86)\Inno Setup 5\"
ISCC.exe "/dMyAppVersion=%1" E:\code\realisim\projects\sargam\installer\windows7\windowsInstaller.iss
popd

REM create output dir and rename sargame.exe to sagam_vx.x.x
mkdir output
move sargam.exe output\sargam_%1.exe
GOTO end

:error
echo needs the version as argument. ex: buildAndCreateInstaller.bat 0.5.3

:end