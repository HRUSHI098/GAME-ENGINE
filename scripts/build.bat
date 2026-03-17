@echo off
setlocal

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Debug

echo [GE] Configuring (%BUILD_TYPE%)...
cmake -S . -B build -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 (echo [GE] Configure failed & exit /b 1)

echo [GE] Building C++ engine...
cmake --build build --config %BUILD_TYPE% --parallel
if errorlevel 1 (echo [GE] Build failed & exit /b 1)

echo [GE] Building C# ScriptCore...
dotnet build ScriptCore\ScriptCore.csproj -c %BUILD_TYPE% --nologo -v q
if errorlevel 1 (echo [GE] ScriptCore build failed & exit /b 1)

echo [GE] Building C# SandboxScripts...
dotnet build Sandbox\Scripts\SandboxScripts.csproj -c %BUILD_TYPE% --nologo -v q
if errorlevel 1 (echo [GE] SandboxScripts build failed & exit /b 1)

echo [GE] Done. Binaries in build\bin\%BUILD_TYPE%\
