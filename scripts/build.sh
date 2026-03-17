#!/bin/bash
set -e

BUILD_TYPE=${1:-Debug}

echo "[GE] Configuring ($BUILD_TYPE)..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=$BUILD_TYPE

echo "[GE] Building C++ engine..."
cmake --build build --config $BUILD_TYPE --parallel $(nproc)

echo "[GE] Building C# ScriptCore..."
dotnet build ScriptCore/ScriptCore.csproj -c $BUILD_TYPE --nologo -v q

echo "[GE] Building C# SandboxScripts..."
dotnet build Sandbox/Scripts/SandboxScripts.csproj -c $BUILD_TYPE --nologo -v q

echo "[GE] Done. Binaries in build/bin/"
