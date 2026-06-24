# Quick Reference Card

## ⚡ Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+Shift+B` | Open task dropdown (select build/run task) |
| `F5` | Start debugging |
| `Ctrl+Shift+P` | Command palette (search for tasks) |
| `Ctrl+`` ` | Open terminal |
| `Ctrl+J` | Toggle terminal visibility |
| `Ctrl+K Ctrl+0` | Focus on editor |
| `Ctrl+K Ctrl+1` | Focus on terminal |
| `Ctrl+P` | Quick file open |
| `Ctrl+Shift+F` | Search in files |

---

## 📋 Most Common Tasks

### Build & Run (Fastest!)
```
Ctrl+Shift+B → "Build & Run Debug" → Enter
```

### Just Build
```
Ctrl+Shift+B → "Build Debug" → Enter
```

### Debug the App
```
F5 → "Debug LibMan (GDB)" → Enter
```

### Release Build
```
Ctrl+Shift+B → "Build Release" → Enter
```

### Run Tests
```
Ctrl+Shift+B → "Tests: Run All Tests" → Enter
```

---

## 🚀 First Time Setup

```
1. Install ZLIB (see docs/getting-started/DEPENDENCIES.md)
2. Ctrl+Shift+B → CMake: Configure Debug
3. Wait for configuration
4. Ctrl+Shift+B → Build & Run Debug
```

---

## 📂 File Guide

| File | Location |
|------|----------|
| Build tasks | `.vscode/tasks.json` |
| Compiler config | `.vscode/settings.json` |
| Debug config | `.vscode/launch.json` |
| Documentation | `docs/INDEX.md` |
| Source code | `src/` |
| Tests | `tests/` |

---

## 💡 Tips & Tricks

✓ **Fastest way to build**: `Ctrl+Shift+B` → Arrow keys → Enter  
✓ **Always build before debugging**: Use F5 (auto-compile)  
✓ **Clear cache if issues**: Delete `build/` folder  
✓ **Run task again**: `Ctrl+P` then type `>Tasks: Run Last Task`  
✓ **Cancel running task**: `Ctrl+C` in terminal  
✓ **Reload VS Code**: `Ctrl+Shift+P` → "Developer: Reload Window"  

---

## 🔍 Troubleshooting Quick Fixes

### Task dropdown not showing?
→ Reload VS Code: `Ctrl+Shift+P` → "Developer: Reload Window"

### Build fails immediately?
→ Check [DEPENDENCIES.md](../getting-started/DEPENDENCIES.md) for ZLIB

### Executable not found?
→ Ensure build succeeds first: `Ctrl+Shift+B` → Build Debug

### Debug won't start?
→ F5 → View `.vscode/launch.json` → Check paths

### Tests won't run?
→ First run: "Tests: Configure Tests" → then build → then run

---

## 📊 Task Groups (Ctrl+Shift+B Dropdown)

### Compilation & Running LibMan
- Build Debug
- Build Release
- Run Debug
- Run Release
- Build & Run Debug
- Build & Run Release

### Tests
- Configure Tests
- Build Tests
- Run All Tests

### CMake Configuration
- CMake: Configure Debug
- CMake: Configure Release

---

## 🎯 Development Workflow

1. **Write code** in `src/` or `tests/`
2. **Build**: `Ctrl+Shift+B` → Build Debug
3. **Test**: Run or Debug (F5)
4. **Fix errors**: Check compiler output
5. **Run tests**: `Ctrl+Shift+B` → Tests: Run All Tests

---

## 📞 Help Resources

- **Setup Issues**: See `docs/setup/VSCODE_SETUP.md`
- **Dependencies**: See `docs/getting-started/DEPENDENCIES.md`
- **Customization**: See `docs/reference/VSCODE_ADVANCED.md`
- **Problems**: See `docs/reference/TROUBLESHOOTING.md`
- **Project file editing**: See `docs/setup/PROJECT_EDITOR.md`
- **All docs**: See `docs/INDEX.md`

---

## 🗂️ LibMan application shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+O` | Open project file |
| `Ctrl+S` | Save project |
| `Ctrl+E` | Edit Project... (Project Editor) |
| — | **File → Import...** — convert GDS / Xschem / Qucs / OAS into CORE views |

Double-click a **layout** view root (`gds`, `oas`, `lstr`, `layout`) to open KLayout. Double-click **schematic** / **symbol** to open Xschem (WSL on Windows). See [KLayout](../setup/KLAYOUT_INTEGRATION.md), [Xschem](../setup/XSCHEM_INTEGRATION.md), and [Import](../setup/IMPORT.md).

---

## ✅ Status Checklist

- [ ] ZLIB installed
- [ ] CMake configured
- [ ] Build successful
- [ ] App runs
- [ ] Debugging works
- [ ] Tests compile
- [ ] Tests run

---

**Pro Tip**: Pin this file to your editor (`Ctrl+K Ctrl+P` while file open) for quick access!

**Back to**: [Documentation Index](../INDEX.md)
