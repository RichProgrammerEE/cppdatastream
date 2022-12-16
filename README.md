# CPPDATASTREAM:
Simple repo exploring the performance of a general-use, block-based processing pipeline.

### Getting setup:
```bash
# Optional (recommended) create + source python virtual environment
python -m venv ./venv
source venv/bin/activate # WINDOWS (venv/Scripts/activate.bat)
python.exe -m pip install --upgrade pip
# Install conan
pip install conan
# Run build script, requires C++ 20 compiler, does a clean build of code
./build.bat
```