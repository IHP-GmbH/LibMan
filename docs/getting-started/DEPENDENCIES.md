# LibMan Project Dependencies - Installation Guide

## Current Status

The LibMan project requires the following to compile:
- ✅ Qt 5.15.2 (configured at `C:/Qt/5.15.2/mingw81_64/lib/cmake/Qt5`)
- ✅ CMake 3.16+
- ✅ MinGW 8.1.0 compiler
- ❌ **ZLIB** (missing - prevents compilation)

## Installing ZLIB

Choose one of the methods below based on your setup:

### **Option 1: Download Pre-built ZLIB for MinGW (Recommended)**

1. Download MinGW-compatible ZLIB from:
   - [GnuWin32 ZLIB](http://gnuwin32.sourceforge.net/packages/zlib.htm)
   - [ZLIB Official](https://zlib.net/)

2. Extract to a known location, e.g.: `C:\zlib-install\`

3. Update `CMakeLists.txt` to point to ZLIB:
   ```cmake
   # Add this before find_package(ZLIB REQUIRED)
   set(ZLIB_ROOT "C:/zlib-install")
   set(ZLIB_INCLUDE_DIR "${ZLIB_ROOT}/include")
   set(ZLIB_LIBRARY "${ZLIB_ROOT}/lib/libz.a")
   ```

4. Or pass via CMake command:
   ```bash
   cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug \
     -DZLIB_INCLUDE_DIR=C:/zlib-install/include \
     -DZLIB_LIBRARY=C:/zlib-install/lib/libz.a
   ```

### **Option 2: Build ZLIB from Source**

```bash
# Download ZLIB source
cd C:\temp
git clone https://github.com/madler/zlib.git
cd zlib

# Configure with MinGW
cmake -B build -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=C:/zlib-install
cmake --build build
cmake --install build

# This creates C:/zlib-install/lib and C:/zlib-install/include
# Then update CMakeLists.txt with paths as shown in Option 1
```

### **Option 3: Using Package Manager (if installed)**

If you have **Chocolatey** installed:
```bash
choco install zlib
```

If you have **MSYS2** installed:
```bash
pacman -S mingw-w64-x86_64-zlib
```

### **Option 4: CMake Approach - Install via vcpkg**

If you want to set up vcpkg (optional one-time setup):

```bash
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install ZLIB
.\vcpkg install zlib:x86-mingw-static

# Then run CMake with vcpkg toolchain
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x86-mingw-static
```

---

## Quick Test After ZLIB Installation

Once ZLIB is installed, verify CMake can find it:

```bash
cd c:\Users\anton\Documents\LibMan
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
```

✅ If successful, you'll see:
```
-- Found ZLIB
-- Configuring done
```

---

## Updating Tasks After Installation

After installing ZLIB:

1. **Clear the old build**:
   ```powershell
   Remove-Item build -Recurse -Force
   mkdir build
   ```

2. **Run first configuration task**:
   - Press `Ctrl+Shift+B` → Select "CMake: Configure Debug"
   - Or manually run the CMake configure command above

3. **Build the project**:
   - Press `Ctrl+Shift+B` → Select "Build Debug"
   - Or use "Build & Run Debug" to compile and run

---

## Troubleshooting

### "Could NOT find ZLIB" error
- Verify ZLIB is installed to the expected location
- Check paths in CMakeLists.txt or CMakePresets.json
- Try specifying paths manually via CMake command line

### CMake still can't find ZLIB after installation
- Add ZLIB path to system `PATH` environment variable
- Restart VS Code after installing ZLIB
- Clear CMake cache: `Remove-Item build -Recurse -Force`

### Wrong ZLIB architecture (32-bit vs 64-bit)
- MinGW 8.1.0 is 32-bit (`mingw810_32`)
- Ensure ZLIB is compiled for x86 (32-bit), not x64
- Check architecture in ZLIB library folder names

---

## Alternative: Temporary Build Without ZLIB

If you want to temporarily skip ZLIB:

Edit `CMakeLists.txt` line 26:
```cmake
# Original:
find_package(ZLIB REQUIRED)

# Change to:
# find_package(ZLIB REQUIRED)  # Temporarily commented
```

**Note**: This may cause linker errors if ZLIB symbols are used in the source code. This is only for testing the configuration.

---

## Support

For more information:
- Check the official CMake error message output
- See `.vscode/tasks.json` for current build configuration
- Review `CMakePresets.json` for CMake preset definitions

---

**Back to**: [Documentation Index](../INDEX.md) | [Quick Start](QUICK_START.md)
