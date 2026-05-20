# VS Code Setup Guide

## Complete Setup Documentation

Your VS Code workspace has been configured with complete build, run, and test task management for the LibMan project.

---

## 📦 What's Configured

### VS Code Configuration Files
- **`tasks.json`** - 10+ build/run/test tasks
- **`settings.json`** - Compiler & CMake config
- **`launch.json`** - Debug configurations
- **`extensions.json`** - 7 recommended extensions

### Features Included
✅ Debug & Release builds  
✅ One-click build + run  
✅ Full GDB debugging support  
✅ Test compilation & execution  
✅ Pre-configured compiler paths  
✅ Code analysis exclusions  
✅ Parallel builds (-j4)  
✅ CMake integration  

---

## 🎯 Available Tasks

### Compilation & Running LibMan (Ctrl+Shift+B)
- Build Debug
- Build Release
- Run Debug
- Run Release
- Build & Run Debug
- Build & Run Release

### Tests (Ctrl+Shift+B)
- Configure Tests
- Build Tests
- Run All Tests

### CMake Configuration (Ctrl+Shift+B)
- CMake: Configure Debug
- CMake: Configure Release

### Debugging (F5)
- Debug LibMan (GDB)
- Debug Tests (GDB)

---

## ⚡ Quick Usage

### Build & Run
1. Press `Ctrl+Shift+B`
2. Select "Build & Run Debug"
3. Press Enter

### Debug
1. Press `F5`
2. Select "Debug LibMan (GDB)"
3. App launches with debugger

### Run Tests
1. Press `Ctrl+Shift+B`
2. Select "Tests: Run All Tests"
3. Tests execute

---

## 🔨 Build Configuration

### Compiler Settings
- **Compiler**: MinGW 8.1.0 (32-bit)
- **Path**: `C:/Qt/Tools/mingw810_32/bin/c++.exe`
- **C++ Standard**: C++17
- **Debugger**: GDB

### CMake Settings
- **Generator**: MinGW Makefiles
- **Build Directory**: `build/`
- **Test Build Directory**: `tests/build/`
- **Parallel Jobs**: 4 (`-j4`)

### Excluded Directories
From explorer and search:
- `build/`
- `tests/build/`
- `.deps/`
- `capnproto/`
- `capnp-install/`
- `lstream/`
- `coverage.*` (coverage files)

---

## 📝 Configuration Files

### `.vscode/tasks.json`
Contains all build and run tasks. Edit to:
- Add new build configurations
- Change compiler flags
- Modify parallel jobs
- Customize task behavior

### `.vscode/settings.json`
Controls VS Code behavior. Edit to:
- Change compiler path
- Modify CMake settings
- Adjust code analysis
- Change editor formatting

### `.vscode/launch.json`
Debug configurations. Edit to:
- Change executable paths
- Add debug arguments
- Modify GDB settings
- Add more debug targets

---

## 🎓 Getting Started

### First Time

1. **Install ZLIB** (see [DEPENDENCIES.md](../getting-started/DEPENDENCIES.md))
2. Configure CMake: `Ctrl+Shift+B` → "CMake: Configure Debug"
3. Build: `Ctrl+Shift+B` → "Build Debug"
4. Run: `Ctrl+Shift+B` → "Run Debug"

### Regular Development

- Build: `Ctrl+Shift+B` (use dropdown)
- Debug: `F5` (starts with automatic build)
- Test: `Ctrl+Shift+B` → "Tests: Run All Tests"

---

## 🔧 Customization

### Increase Build Speed
Edit `.vscode/tasks.json`, find build tasks, change `-j4` to `-j8`:
```json
"args": ["--build", "build", "--config", "Debug", "-j8"]
```

### Use Different Generator
In `.vscode/tasks.json`, change `"MinGW Makefiles"` to:
- `"Ninja"` (requires Ninja installed)
- `"Visual Studio 16 2019"`

### Change Compiler
Edit `.vscode/settings.json`:
```json
"C_Cpp.default.compilerPath": "path/to/your/compiler"
```

---

## 📚 Related Documentation

- **Quick Start**: [Quick Start Guide](../getting-started/QUICK_START.md)
- **Dependencies**: [Install ZLIB](../getting-started/DEPENDENCIES.md)
- **Troubleshooting**: [Troubleshooting Guide](../reference/TROUBLESHOOTING.md)
- **Advanced**: [Advanced Customization](../reference/VSCODE_ADVANCED.md)
- **Reference**: [Quick Reference Card](../reference/QUICK_REFERENCE.md)

---

**Back to**: [Documentation Index](../INDEX.md)
