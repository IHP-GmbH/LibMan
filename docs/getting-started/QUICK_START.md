# Quick Start Guide

**⏱️ 5 minute setup | No prior knowledge needed**

---

## What is LibMan?

LibMan is a Qt5-based application for managing library components. It's built with:
- **Language**: C++17
- **Framework**: Qt 5.15.2
- **Build System**: CMake
- **Compiler**: MinGW 8.1.0

---

## ✅ Prerequisites (Check These)

- [ ] CMake 3.16+ installed
- [ ] MinGW 8.1.0 compiler available
- [ ] Qt 5.15.2 development libraries
- [ ] ZLIB library (see next step)

---

## 🚀 Step 1: Install ZLIB (Required!)

This is the **only blocking requirement**.

### Fastest Option (Recommended):

**If you have MSYS2:**
```bash
pacman -S mingw-w64-x86_64-zlib
```

**If you have Chocolatey:**
```bash
choco install zlib
```

**Otherwise:** See [DEPENDENCIES.md](DEPENDENCIES.md) for 4 more options.

---

## 🔨 Step 2: Configure & Build

The first build also clones and compiles **Cap'n Proto** (several minutes). See [Build guide](../BUILD.md) for qmake, Windows, and CI details.

### Open Terminal:
```bash
cd c:\Users\anton\Documents\LibMan
```

### Configure CMake:
```bash
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
```

✅ Should show: `-- Configuring done`

### Build:
```bash
cmake --build build --config Debug -j4
```

✅ Should complete without errors

---

## ▶️ Step 3: Run It!

### From Terminal:
```bash
./build/src/LibMan.exe
```

### Or from VS Code:
Press `Ctrl+Shift+B` → Select "Build & Run Debug"

---

## 🐛 Step 4: Debug

Press `F5` in VS Code to start debugging.

---

## ✅ Success!

If you see the LibMan window, you're done! 🎉

---

## 📚 Next Steps

- **Learn VS Code tasks**: [Quick Reference](../reference/QUICK_REFERENCE.md)
- **Advanced setup**: [VS Code Setup Guide](../setup/VSCODE_SETUP.md)
- **Customize build**: [Advanced Customization](../reference/VSCODE_ADVANCED.md)
- **Run tests**: [See Troubleshooting](../reference/TROUBLESHOOTING.md)

---

## ❌ Not Working?

1. **ZLIB errors**: See [DEPENDENCIES.md](DEPENDENCIES.md)
2. **Build errors**: Check [Troubleshooting](../reference/TROUBLESHOOTING.md)
3. **VS Code issues**: See [VS Code Setup](../setup/VSCODE_SETUP.md)

---

## 💡 Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Ctrl+Shift+B` | Build & Run |
| `F5` | Start Debugging |
| `Ctrl+`` ` | Open Terminal |

---

**Still stuck?** Go back to [Documentation Index](../INDEX.md)
