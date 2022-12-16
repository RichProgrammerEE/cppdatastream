@REM @echo off
@REM Tell conan to use Ninja generator
set CONAN_CMAKE_GENERATOR=Ninja
@REM Install dependencies
rm -rf deps
conan install . --install-folder=deps --build=missing
@REM Build the project
set BUILD_TYPE=Release
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_INSTALL_PREFIX=install -GNinja
cmake --build build --config %BUILD_TYPE% --clean-first -v