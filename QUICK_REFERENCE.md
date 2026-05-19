# LibMan VS Code - Quick Reference Card

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

---

## 📋 Most Common Tasks

### **Quick Build & Run**
1. Press `Ctrl+Shift+B`
2. Choose: `Compilation & Running LibMan: Build & Run Debug`
3. Press Enter

### **Build Only**
1. Press `Ctrl+Shift+B`
2. Choose: `Compilation & Running LibMan: Build Debug`
3. Press Enter

### **Debug the App**
1. Press `F5`
2. Choose: `Debug LibMan (GDB)`
3. Press Enter (builds automatically)

### **Run Tests**
1. Press `Ctrl+Shift+B`
2. Choose: `Tests: Run All Tests`
3. Press Enter

### **Release Build**
1. Press `Ctrl+Shift+B`
2. Choose: `Compilation & Running LibMan: Build Release`
3. Press Enter

---

## 🚀 First Time Setup

```
1. Install ZLIB (see DEPENDENCIES.md)
2. Press Ctrl+Shift+B → CMake: Configure Debug
3. Wait for configuration to complete
4. Press Ctrl+Shift+B → Build & Run Debug
```

---

## 📂 File Guide

| File | Purpose |
|------|---------|
| `.vscode/tasks.json` | All build/run/test tasks |
| `.vscode/settings.json` | Compiler and CMake config |
| `.vscode/launch.json` | Debug configurations |
| `CMakePresets.json` | CMake build presets |
| `VSCODE_SETUP.md` | Full setup documentation |
| `DEPENDENCIES.md` | How to install ZLIB |
| `VSCODE_ADVANCED.md` | Advanced customization |

---

## 🎯 Task Dropdown Groups

### Compilation & Running LibMan
- Build Debug
- Build Release
- Run Debug
- Run Release
- Build & Run Debug
- Build & Run Release

### Tests
- Configure Tests (first time)
- Build Tests
- Run All Tests

---

## 💡 Tips & Tricks

✓ **Fastest way to build**: `Ctrl+Shift+B` → Arrow keys → Enter  
✓ **Always build before debugging**: Use F5 (automatic pre-build)  
✓ **Clear cache if issues**: Delete `build/` folder  
✓ **Run task again**: Press `Ctrl+P` then `>Tasks: Run Last Task`  
✓ **Cancel running task**: `Ctrl+C` in terminal  

---

## 🔍 Troubleshooting Quick Fixes

### Task dropdown not showing?
→ Reload VS Code: `Ctrl+Shift+P` → "Developer: Reload Window"

### Build fails immediately?
→ Check DEPENDENCIES.md for ZLIB installation

### Executable not found?
→ Ensure build succeeds first: `Ctrl+Shift+B` → Build Debug

### Debug won't start?
→ F5 → View launch.json → Check paths are correct

### Tests won't run?
→ First run: `Tests: Configure Tests` → then `Tests: Build Tests` → then `Tests: Run All Tests`

---

## 📞 Help Resources

- **Setup Issues**: See `VSCODE_SETUP.md`
- **Missing Dependencies**: See `DEPENDENCIES.md`
- **Customize Tasks**: See `VSCODE_ADVANCED.md`
- **CMake Help**: Run task with `--verbose` flag
- **Project Info**: Check `CMakeLists.txt` and `.pro` files

---

## ✅ Status Checklist

- [ ] ZLIB installed (run DEPENDENCIES.md steps)
- [ ] CMake configured (CMake: Configure Debug task)
- [ ] Build successful (Build Debug task)
- [ ] App runs (Run Debug task)
- [ ] Tests compile (Tests: Build Tests task)
- [ ] Debugging works (F5 key)

---

**Pro Tip**: Pin this file to your editor (`Ctrl+K Ctrl+P` while file open) for quick access!
