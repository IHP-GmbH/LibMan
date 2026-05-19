# VS Code Tasks - Advanced Configuration Guide

## Task Structure Overview

All tasks are organized in `.vscode/tasks.json` with three main categories:

### 1. **CMake Configuration Tasks** (Initial Setup)
- `CMake: Configure Debug`
- `CMake: Configure Release`

These prepare the build environment and must run before compilation.

### 2. **Compilation & Running LibMan** (Main Development)
Organized in a dropdown group with prefix `Compilation & Running LibMan:`
- `Build Debug` - Compiles in Debug mode
- `Build Release` - Compiles in Release mode  
- `Run Debug` - Runs the debug executable
- `Run Release` - Runs the release executable
- `Build & Run Debug` - Combined build and run
- `Build & Run Release` - Combined build and run

### 3. **Tests** (Testing Group)
Organized in a dropdown group with prefix `Tests:`
- `Configure Tests` - One-time CMake setup for tests
- `Build Tests` - Compiles test suite
- `Run All Tests` - Executes tests via CTest

---

## Customizing Tasks

### Changing Compiler

Edit `.vscode/settings.json`:
```json
"C_Cpp.default.compilerPath": "C:/path/to/your/compiler/bin/c++.exe"
```

### Adjusting Parallel Build Jobs

In `.vscode/tasks.json`, find the build tasks and change `-j4` to your desired count:
```json
"args": [
  "--build", "build",
  "--config", "Debug",
  "-j8"  // Change 4 to 8 or desired number
]
```

Recommendation:
- `-j2`: Slow systems or 2-core CPU
- `-j4`: Standard (recommended for most systems)
- `-j8`: Powerful workstations (8+ cores)

### Adding a New Build Configuration

Example: Adding an "AddressSanitizer" debug build

1. Copy a build task in `tasks.json`
2. Modify the label and args:

```json
{
  "label": "Compilation & Running LibMan: Build AddressSanitizer",
  "type": "shell",
  "command": "cmake",
  "args": [
    "-B", "build-asan",
    "-G", "MinGW Makefiles",
    "-DCMAKE_BUILD_TYPE=Debug",
    "-DCMAKE_CXX_FLAGS=-fsanitize=address"
  ],
  "options": { "cwd": "${workspaceFolder}" },
  "group": { "kind": "build", "isDefault": false },
  "problemMatcher": "$gcc"
}
```

### Using a Different Generator

Change generator in all CMake tasks:
```json
"-G", "Ninja"  // Replace "MinGW Makefiles"
```

Available generators for Windows:
- "MinGW Makefiles" (current)
- "Ninja" (requires Ninja installed)
- "NMake Makefiles" (requires NMake/MSVC)
- "Visual Studio 16 2019"
- "Visual Studio 17 2022"

---

## Task Execution Methods

### Method 1: Quick Build (Recommended)
Press `Ctrl+Shift+B` → Select from dropdown → Press Enter

### Method 2: Command Palette
1. Press `Ctrl+Shift+P`
2. Type: "Tasks: Run Task"
3. Select desired task
4. Press Enter

### Method 3: Terminal
```bash
# Using npm-style run in terminal:
# Go to: Terminal → Run Task

# Or manually run from terminal:
cmake --build build --config Debug -j4
```

### Method 4: Keyboard Shortcuts (Custom)
Add to `.vscode/keybindings.json`:
```json
{
  "key": "ctrl+alt+b",
  "command": "workbench.action.tasks.runTask",
  "args": "Compilation & Running LibMan: Build Debug"
}
```

---

## Problem Matchers

Tasks use problem matchers to parse compiler output:

| Matcher | Purpose | Compiler |
|---------|---------|----------|
| `$gcc` | GCC/Clang errors | MinGW, GCC, Clang |
| `$cmake` | CMake configuration errors | CMake |
| `$msCompile` | MSVC errors | Visual Studio |

To add custom patterns, edit the `"problemMatcher"` field.

---

## Pre-Launch and Dependent Tasks

Example: Debug task runs build first

```json
"preLaunchTask": "Compilation & Running LibMan: Build Debug"
```

Example: Test task depends on build
```json
"dependsOn": ["Tests: Build Tests"]
```

---

## Presentation Options

Customize how tasks display output:

```json
"presentation": {
  "echo": true,           // Show the command being run
  "reveal": "always",     // Always show terminal panel
  "focus": false,         // Keep focus on editor
  "panel": "shared"       // Share terminal across tasks
}
```

Options for `reveal`:
- `"always"` - Always show terminal
- `"silent"` - Never show unless error
- `"new"` - Open new terminal each time

---

## Environment Variables

Pass environment variables to tasks:

```json
"options": {
  "cwd": "${workspaceFolder}",
  "env": {
    "CMAKE_PREFIX_PATH": "C:/Qt/5.15.2/mingw81_64",
    "ZLIB_ROOT": "C:/zlib-install"
  }
}
```

Useful variables:
- `${workspaceFolder}` - Root project directory
- `${workspaceRootFolderName}` - Folder name only
- `${file}` - Currently open file
- `${fileDirname}` - Directory of current file

---

## Running Tests Automatically

### After Every Build
Create a combined task:

```json
{
  "label": "Compilation & Running LibMan: Build Debug & Run Tests",
  "type": "shell",
  "command": "cmake --build build --config Debug -j4 && cd tests/build && ctest --output-on-failure",
  "options": { "cwd": "${workspaceFolder}" },
  "group": { "kind": "build" },
  "problemMatcher": "$gcc"
}
```

### Continuous Testing (Watch Mode)
Create a file watcher task (not directly supported in tasks.json):
- Use VS Code extension: "Task Buttons" or "Run on Save"

---

## Debugging Tasks

### View Task Execution Output
1. Run any task
2. Check the "Terminal" panel (Ctrl+`)
3. See full command output

### Troubleshoot "Command not found"
- Ensure command is in system `PATH`
- Use full path in command field
- Check working directory in `options.cwd`

### Debugging CMake Issues
Add verbose output:
```json
"args": [
  "--build", "build",
  "--config", "Debug",
  "--verbose"  // Add this
]
```

---

## Performance Optimization

### Speed Up Builds
1. Increase parallel jobs: `-j8` or `-j12`
2. Use faster linker:
   ```json
   "-DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld"
   ```
3. Use precompiled headers (in CMakeLists.txt)

### Speed Up CMake Configuration
```json
"-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"  // Already in config
```

---

## Testing Multiple Configurations

Run Debug, Release, and Tests in sequence:

1. Copy a task and modify
2. Use task dependencies:

```json
{
  "label": "Full Test Suite",
  "dependsOn": [
    "Compilation & Running LibMan: Build Debug",
    "Compilation & Running LibMan: Build Release",
    "Tests: Run All Tests"
  ],
  "problemMatcher": []
}
```

---

## Restoring Default Configuration

If tasks get corrupted:

1. **Backup current**: Copy `.vscode/tasks.json`
2. **Restore from VSCODE_SETUP.md**: Contains full default configuration
3. **Or use Git**: `git checkout .vscode/tasks.json`

---

## Useful Extensions for Task Management

1. **Task Buttons** - Quick task launcher in toolbar
2. **Task Runner** - Enhanced task management
3. **Command Runner** - Run any command from status bar
4. **Run on Save** - Auto-run tasks on file changes

Install from VS Code Extensions marketplace.
