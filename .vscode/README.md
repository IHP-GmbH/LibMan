# LibMan VS Code Tasks Configuration

This directory contains VS Code configuration files for building and debugging the LibMan project.

## Files

### tasks.json
Defines all build, run, and test tasks with organized groups:

#### **Compilation & Running LibMan** (Dropdown Tasks)
- **Build Debug** - Compiles LibMan in Debug mode
- **Build Release** - Compiles LibMan in Release mode  
- **Run Debug** - Launches the debug executable
- **Run Release** - Launches the release executable
- **Build & Run Debug** - Compiles and runs in Debug mode
- **Build & Run Release** - Compiles and runs in Release mode

#### **Tests** (Dropdown Tasks)
- **Configure Tests** - Configures the test project using CMake
- **Build Tests** - Compiles all tests
- **Run All Tests** - Executes all tests with output

#### **CMake Configuration** (Support Tasks)
- **Configure Debug** - Initial CMake setup for Debug build
- **Configure Release** - Initial CMake setup for Release build

### settings.json
VS Code editor settings including:
- C++ compiler path and standard configuration
- CMake build directory settings
- Code analysis exclusions (build folders, dependencies, etc.)
- Editor formatting rules

### launch.json
Debug configurations for:
- **Debug LibMan (GDB)** - Debug the main LibMan application
- **Debug Tests (GDB)** - Debug the test suite

### extensions.json
Recommended VS Code extensions for C++ development with CMake.

## Quick Start

### First Time Setup
1. Run task: **CMake: Configure Debug** (or Release)
2. Once configuration is complete, you can use any compilation/run tasks

### Build LibMan
- Press `Ctrl+Shift+B` and select from the dropdown
- Common options: Build Debug, Build & Run Debug

### Run Tests
- Run task: **Tests: Run All Tests**
- To build and run in one step

### Debug
- Press `F5` to start debugging (uses the configured debug task)
- Use **Debug LibMan (GDB)** or **Debug Tests (GDB)** configurations

## Requirements

Before building, ensure you have installed:
- CMake (version 3.16 or higher)
- MinGW compiler (mingw810_32 configured in settings)
- Qt 5.15.2 development libraries
- ZLIB development libraries
- All dependencies listed in CMakeLists.txt

## Dependencies Installation (if needed)

For ZLIB on Windows with MinGW:
```bash
# Using vcpkg
vcpkg install zlib:x86-mingw-static

# Or download and build from source
```

## Troubleshooting

### CMake Configuration Fails
- Ensure all dependencies are installed
- Verify Qt5 path in CMakeLists.txt matches your installation
- Clear build directory and reconfigure: `Remove-Item build -Recurse -Force`

### Compilation Errors
- Check that C++ standard is set to C++17 (configured in settings)
- Verify MinGW compiler path in settings.json
- Check for conflicting CMake generators

### Tests Won't Run
- Ensure tests are properly configured before running
- Check that test executable path is correct in launch.json
