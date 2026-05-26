# Directory Structure

## Project Layout

```
LibMan/
├── README.md                    ← Main project README
├── CMakeLists.txt              ← CMake configuration
├── CMakePresets.json           ← CMake presets
├── libman.pro                  ← QtCreator project file
│
├── 📁 .vscode/                 ← VS Code configuration
│   ├── tasks.json              ← Build/Run/Test tasks
│   ├── settings.json           ← Compiler & CMake settings
│   ├── launch.json             ← Debug configurations
│   ├── extensions.json         ← Recommended extensions
│   └── README.md               ← VS Code quick reference
│
├── 📁 docs/                    ← All documentation
│   ├── INDEX.md                ← Documentation hub (START HERE)
│   │
│   ├── getting-started/        ← For new users
│   │   ├── QUICK_START.md      ← 5 minute setup
│   │   ├── DEPENDENCIES.md     ← Install ZLIB
│   │   └── SETUP_VERIFICATION.md ← Verify your setup
│   │
│   ├── setup/                  ← Setup guides
│   │   ├── VSCODE_SETUP.md     ← VS Code configuration
│   │   └── DIRECTORY_STRUCTURE.md ← This file
│   │
│   └── reference/              ← Reference & advanced
│       ├── QUICK_REFERENCE.md  ← Keyboard shortcuts
│       ├── VSCODE_ADVANCED.md  ← Customization
│       └── TROUBLESHOOTING.md  ← Common issues
│
├── 📁 src/                     ← Source code
│   ├── main.cpp
│   ├── mainwindow.h/cpp
│   └── ... other source files
│
├── 📁 tests/                   ← Test suite
│   ├── tests.pro               ← QtCreator test project
│   ├── main.cpp
│   ├── tst_*.cpp/h             ← Test files
│   └── build/                  ← Test build output
│
├── 📁 build/                   ← Main build output
│   ├── src/LibMan.exe          ← Executable
│   ├── CMakeFiles/
│   ├── Makefile
│   └── compile_commands.json
│
├── 📁 icons/                   ← Application icons
├── 📁 pics/                    ← Images/resources
├── 📁 gds/                     ← GDS file handling
├── 📁 oas/                     ← OAS file handling
├── 📁 lstream/                 ← LStream file handling
├── 📁 extension/               ← Qt extensions
├── 📁 QtPropertyBrowser/       ← Property browser widget
│
├── 📁 .deps/lstream/           ← LStream schema repo (cloned on first build)
├── 📁 capnproto/               ← Cap'n Proto sources (cloned; gitignored)
├── 📁 capnp-install/           ← Cap'n Proto install prefix + capnp_install_stamp
├── 📁 capnp/                   ← Generated schema C++ (.capnp.cc / .h)
│
└── ... other files (coverage.*, scripts, installer, etc.)
```

---

## Documentation Organization

### 📚 docs/ (Main Documentation Hub)

**Location**: `docs/INDEX.md`  
**Purpose**: Central navigation for all documentation

### 🚀 docs/getting-started/ (For New Users)

| File | Purpose |
|------|---------|
| `QUICK_START.md` | 5-minute setup guide |
| `DEPENDENCIES.md` | How to install ZLIB |
| `SETUP_VERIFICATION.md` | Verify your setup |

**When to use**: First time using LibMan

### ⚙️ docs/setup/ (Setup Guides)

| File | Purpose |
|------|---------|
| `VSCODE_SETUP.md` | VS Code configuration details |
| `DIRECTORY_STRUCTURE.md` | File organization (this file) |

**When to use**: Need detailed setup info

### 📖 docs/reference/ (Reference & Advanced)

| File | Purpose |
|------|---------|
| `QUICK_REFERENCE.md` | Shortcuts, common tasks |
| `VSCODE_ADVANCED.md` | Advanced customization |
| `TROUBLESHOOTING.md` | Common issues & fixes |

**When to use**: Development and customization

---

## Build Directories

### Main Build: `build/`
```
build/
├── src/
│   ├── LibMan.exe        ← Executable (after build)
│   └── CMakeFiles/
├── CMakeFiles/
├── CMakeCache.txt
├── Makefile
└── compile_commands.json ← For IDE integration
```

Created when: First CMake configuration  
Used for: Building LibMan application

### Test Build: `tests/build/`
```
tests/build/
├── CMakeFiles/
├── CMakeCache.txt
├── Makefile
├── ctest executable   ← Test runner
└── *.exe             ← Individual test executables
```

Created when: First test configuration  
Used for: Building and running tests

---

## Hidden/Excluded Directories

These folders are hidden from VS Code Explorer:

| Directory | Purpose | Why Hidden |
|-----------|---------|-----------|
| `build/` | Main build artifacts | Large, often regenerated |
| `tests/build/` | Test build artifacts | Large, often regenerated |
| `.deps/` | External dependencies | Auto-generated, not needed |
| `capnproto/` | Cap'n Proto git clone | Gitignored; see [BUILD.md](../BUILD.md) |
| `capnp-install/` | Cap'n Proto install + stamp | Gitignored |
| `capnp/` | Generated Cap'n Proto / LStream C++ | Gitignored (sources committed under `capnp/` may exist until regen) |
| `.deps/lstream/` | LStream schema repository | Gitignored |
| `lstream/` | LStream C++ integration (project source) | In repo |
| `coverage.*` | Coverage report files | Generated files |

---

## Source Organization

### Application Code: `src/`
Main application source files:
- `mainwindow.h/cpp` - Main UI window
- `main.cpp` - Entry point
- `*.h/cpp` - Other components

### Tests: `tests/`
Test suite organized by component:
- `tst_*.cpp` - Test files
- `tests.pro` - Qt test project

### Supporting Libraries
- `gds/` - GDS file format support
- `oas/` - OAS file format support
- `lstream/` - LStream file format support
- `extension/` - Qt property browser extensions
- `QtPropertyBrowser/` - Qt widget library

---

## Configuration Files

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Main CMake project file |
| `CMakePresets.json` | CMake build presets |
| `libman.pro` | QtCreator project file |
| `Doxyfile` | Doxygen documentation config |

---

## Key File Paths

```
Source:          src/main.cpp
Headers:         src/*.h
Tests:           tests/tst_*.cpp
Build output:    build/src/LibMan.exe
Test executable: tests/build/tst_*.exe
Documentation:   docs/INDEX.md
VS Code config:  .vscode/tasks.json
CMake config:    CMakeLists.txt
```

---

## Accessing Files in VS Code

### Quick File Navigation
- `Ctrl+P` - Open file by name
- `Ctrl+Shift+P` - Command palette
- `Ctrl+Shift+F` - Search in files

### Excluded from Search
These don't appear in file search:
- `build/`
- `.deps/`
- `capnproto/`
- `lstream/`
- `coverage.*`

---

**Back to**: [Documentation Index](../INDEX.md) | [VS Code Setup](VSCODE_SETUP.md)
