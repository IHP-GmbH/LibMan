# LibMan Build & Test Tasks - Setup Summary

## ✅ Completed Setup

Your VS Code workspace has been configured with complete build, run, and test task management for the LibMan project.

### Created Files

1. **`.vscode/tasks.json`** - Main task configuration with 10+ tasks organized in 3 dropdown groups
2. **`.vscode/settings.json`** - VS Code C++ and CMake settings  
3. **`.vscode/launch.json`** - Debug configurations for LibMan and Tests
4. **`.vscode/extensions.json`** - Recommended VS Code extensions
5. **`.vscode/README.md`** - Full documentation of the setup

---

## 📋 Available Tasks

### **Compilation & Running LibMan** (Dropdown Group)
Access these by:
- Press `Ctrl+Shift+B` (Run Build Task)
- Select from dropdown menu

| Task | Purpose |
|------|---------|
| Build Debug | Compile LibMan in Debug mode |
| Build Release | Compile LibMan in Release mode |
| Run Debug | Launch debug executable |
| Run Release | Launch release executable |
| Build & Run Debug | Compile + run in one command |
| Build & Run Release | Compile + run in one command |

### **Tests** (Dropdown Group)
Access these by:
- Press `Ctrl+Shift+B` then select Tests group
- Or use Command Palette: Tasks: Run Task

| Task | Purpose |
|------|---------|
| Configure Tests | Initialize CMake for tests folder (runs first) |
| Build Tests | Compile test suite |
| Run All Tests | Execute all tests via CTest |

### **CMake Configuration** (Support Tasks)
Run once before first build:
- CMake: Configure Debug
- CMake: Configure Release

---

## 🎯 How to Use

### **First Time Build**
1. Open Terminal: `Ctrl+`` `
2. Run: `cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug`
3. Once complete, all other tasks will work

### **Regular Building**
- Press `Ctrl+Shift+B` → Select "Build Debug" or "Build Release"

### **Build & Run Immediately**
- Press `Ctrl+Shift+B` → Select "Build & Run Debug" or "Build & Run Release"

### **Run Tests**
1. First time only: Run task "Tests: Configure Tests"
2. Then: Run task "Tests: Build Tests"  
3. Finally: Run task "Tests: Run All Tests"

### **Debug the Application**
- Press `F5` → Select "Debug LibMan (GDB)"
- Automatically builds in Debug mode first

### **Debug Tests**
- Press `F5` → Select "Debug Tests (GDB)"
- Automatically builds tests first

---

## ⚙️ Configuration Details

### Compiler & Tools
- **Generator**: MinGW Makefiles
- **Compiler**: MinGW 8.1.0 (C:/Qt/Tools/mingw810_32/bin/c++.exe)
- **Debugger**: GDB (C:/Qt/Tools/mingw810_32/bin/gdb.exe)
- **C++ Standard**: C++17
- **Qt Version**: 5.15.2

### Build Directories
- Main project: `build/` (Debug & Release)
- Tests: `tests/build/` (Debug only)

### Excluded from Search & Analysis
- `build/` and `tests/build/` directories
- Dependencies: `.deps/`, `capnproto/`, `capnp-install/`, `lstream/`

---

## 📝 Next Steps

### Important: Handle Missing Dependencies
The project requires these to be installed:
1. **ZLIB** - Add ZLIB development libraries if missing
2. **Qt 5.15.2** - Already referenced in CMakeLists.txt
3. **CMake 3.16+** - For build system

If ZLIB is missing, install via:
```bash
# Using vcpkg (if available)
vcpkg install zlib:x86-mingw-static

# Or via package manager on your system
```

### Customize Tasks
Edit `.vscode/tasks.json` to:
- Change compiler path if different from MinGW
- Adjust parallel jobs (currently `-j4`)
- Add additional build configurations
- Modify test executable paths

### Use CMake Extension
For visual CMake configuration:
1. Install "CMake Tools" by Microsoft
2. Use the CMake sidebar to configure/build
3. Tasks still work alongside CMake Tools

---

## 🔧 Troubleshooting

### Tasks Don't Appear
- Reload VS Code: `Ctrl+Shift+P` → "Developer: Reload Window"
- Check `.vscode/tasks.json` for syntax errors

### Build Fails with "ZLIB not found"
- Install ZLIB development libraries
- Update CMakeLists.txt path if needed

### Executable Not Found When Running
- First ensure build is successful
- Verify path in task definition matches actual executable location
- Check build folder has the executable

### Debug Session Won't Start
- Ensure "Build Debug" task completes successfully first
- Verify GDB path is correct in settings
- Check launch.json for correct executable paths

---

## 📚 File Locations for Reference

```
.vscode/
├── tasks.json          ← Edit to customize build/test tasks
├── settings.json       ← C++ compiler and CMake configuration
├── launch.json         ← Debug configurations
├── extensions.json     ← Recommended extensions
└── README.md          ← Detailed documentation
```

---

## ✨ Features Included

✅ Debug and Release build configurations  
✅ Dropdown task selection for easy access  
✅ Automatic test discovery and running  
✅ GDB debugging support  
✅ Parallel build jobs (-j4)  
✅ Excluded build/dependency folders from search  
✅ Problem matcher for error highlighting  
✅ Pre-launch tasks that build before debugging  

---

**Setup Complete! You can now use `Ctrl+Shift+B` to access all build tasks.**
