# Troubleshooting Guide

## Common Issues & Solutions

---

## 🔴 Build Issues

### Error: "Could NOT find ZLIB"

**Symptoms**: CMake fails with ZLIB not found

**Solution**:
1. Follow [DEPENDENCIES.md](../getting-started/DEPENDENCIES.md)
2. Install ZLIB using one of 4 methods
3. Clear build: `Remove-Item build -Recurse -Force`
4. Reconfigure: `Ctrl+Shift+B` → CMake: Configure Debug

**Time**: 5-20 minutes

---

### Error: "CMAKE_CXX_COMPILER not set"

**Symptoms**: CMake fails with compiler error

**Solution**:
1. Check MinGW is installed: `c++.exe --version`
2. Verify path in `.vscode/settings.json`
3. Clear build and reconfigure
4. Reload VS Code

**Time**: 5 minutes

---

### Build takes very long

**Solution** (increase parallel jobs):
1. Edit `.vscode/tasks.json`
2. Find build tasks, change `-j4` to `-j8` or higher
3. Save and rebuild

**Note**: Increase based on CPU cores (2x cores is typical)

**Time**: 2 minutes

---

## 🔴 VS Code Issues

### Task dropdown is empty

**Symptoms**: `Ctrl+Shift+B` doesn't show tasks

**Solution**:
1. Reload VS Code: `Ctrl+Shift+P` → "Developer: Reload Window"
2. Wait 5 seconds for tasks.json to load
3. Check `.vscode/tasks.json` for syntax errors
4. Restart VS Code completely

**Time**: 1 minute

---

### No IntelliSense or code completion

**Symptoms**: No autocomplete, no error highlighting

**Solution**:
1. Ensure C++ extension is installed
2. Wait for IntelliSense indexing (can take a minute)
3. Check `compile_commands.json` exists in `build/`
4. Rebuild: `Ctrl+Shift+B` → Build Debug

**Time**: 2-5 minutes

---

### Debug won't start (F5)

**Symptoms**: Pressing F5 does nothing or shows error

**Solution**:
1. Build first: `Ctrl+Shift+B` → Build Debug
2. Check executable exists: `build/src/LibMan.exe`
3. Verify `.vscode/launch.json` paths
4. Check GDB path: `C:/Qt/Tools/mingw810_32/bin/gdb.exe` exists
5. Reload VS Code

**Time**: 5 minutes

---

## 🔴 Compilation Errors

### Error: "undefined reference to `ZLIB...`"

**Symptoms**: Linker error mentioning ZLIB

**Solution**:
1. ZLIB wasn't found during CMake
2. Follow [DEPENDENCIES.md](../getting-started/DEPENDENCIES.md)
3. Clear build and reconfigure
4. Rebuild

**Time**: 10-20 minutes

---

### Error: "mingw810_32 not found"

**Symptoms**: Compiler path error

**Solution**:
1. Verify MinGW installation: `ls C:/Qt/Tools/mingw810_32/`
2. Update path in `.vscode/settings.json`
3. Use actual installed compiler path
4. Reload VS Code

**Time**: 5 minutes

---

### Error: "Qt5 not found"

**Symptoms**: CMake can't find Qt5

**Solution**:
1. Verify Qt installation: `ls C:/Qt/5.15.2/mingw81_64/`
2. Check CMakeLists.txt line ~24 for Qt5_DIR path
3. Update if path is different
4. Clear build and reconfigure

**Time**: 5 minutes

---

## 🔴 Execution Issues

### Executable not found when running

**Symptoms**: "File not found" error on Run task

**Solution**:
1. Build first: `Ctrl+Shift+B` → Build Debug
2. Verify build succeeds (no errors)
3. Check executable exists: `build/src/LibMan.exe`
4. Update `.vscode/tasks.json` run task if path is wrong

**Time**: 5 minutes

---

### App crashes immediately

**Symptoms**: Window appears then closes

**Solution**:
1. Debug instead: Press `F5`
2. Check console output for error
3. Verify Qt libraries are found
4. Check all dependencies installed

**Time**: 10 minutes

---

## 🔴 Test Issues

### Tests won't configure

**Symptoms**: "Tests: Configure Tests" fails

**Solution**:
1. Main project must build first
2. Build: `Ctrl+Shift+B` → Build Debug
3. Then: `Ctrl+Shift+B` → Tests: Configure Tests
4. Clear `tests/build` if issues persist

**Time**: 5 minutes

---

### Tests won't compile

**Symptoms**: "Tests: Build Tests" fails

**Solution**:
1. Configure first: `Ctrl+Shift+B` → Tests: Configure Tests
2. Check test source files in `tests/`
3. Verify CMakeLists.txt exists in `tests/`
4. Check for missing test includes

**Time**: 5-10 minutes

---

### Tests won't run

**Symptoms**: "Tests: Run All Tests" shows no tests

**Solution**:
1. Build first: `Ctrl+Shift+B` → Tests: Build Tests
2. Check test executable exists
3. Try running manually: `tests/build/tst_*.exe`
4. Verify test discovery works

**Time**: 5 minutes

---

## 🔴 Terminal Issues

### Terminal not opening

**Symptoms**: Can't open terminal with `Ctrl+`` `

**Solution**:
1. Use menu: Terminal → New Terminal
2. Or press `Ctrl+J` to toggle
3. Try `Ctrl+Shift+P` → "Terminal: Create New Terminal"

**Time**: 1 minute

---

### Commands not found in terminal

**Symptoms**: "cmake: command not found"

**Solution**:
1. Add to PATH environment variable
2. Restart VS Code after PATH change
3. Or use full path: `C:\path\to\cmake.exe`

**Time**: 5 minutes

---

## 🔴 File/Config Issues

### Files still showing in explorer

**Symptoms**: Excluded files still visible (build/, coverage.*)

**Solution**:
1. Reload VS Code: `Ctrl+Shift+P` → "Developer: Reload Window"
2. Check `.vscode/settings.json` for exclusions
3. Verify syntax is correct
4. Clear VSCode cache if needed

**Time**: 1 minute

---

### Can't find documentation

**Symptoms**: Files like DEPENDENCIES.md not visible

**Solution**:
1. They're now in `docs/` folder
2. Start with `docs/INDEX.md`
3. Navigate to `docs/getting-started/`
4. Use `Ctrl+P` to search for files

**Time**: 1 minute

---

## 🟡 Performance Issues

### Build is slow

**Solution**:
1. Increase parallel jobs (see "Build takes very long" above)
2. Use Release mode for faster execution
3. Check disk space
4. Disable antivirus scanning of build folder

**Time**: 5-10 minutes

---

### VS Code is sluggish

**Solution**:
1. Exclude more folders from search (see `.vscode/settings.json`)
2. Close unnecessary files/tabs
3. Disable heavy extensions
4. Increase VS Code memory limit

**Time**: 5 minutes

---

## 🟡 LibMan runtime issues

### View still shown after deleting it in Project Editor

**Cause (fixed):** Old `LIBRARY_*` property keys were not cleared on project reload.

**If it happens on an old build:** Upgrade LibMan, save again from Project Editor, or **File → Open** the project file to force a full reload.

---

### New view from Browse does not appear in LibMan

**Symptoms:** Row added in Project Editor but View tree has no `lstr` / new view.

**Check:**
1. **Library** column is filled (Browse copies the name from the row above if empty).
2. Path points to an existing file.
3. You clicked **File → Save** in Project Editor (or main **File → Save**).

---

### Browse inserted a short relative path; view missing

**Cause (fixed):** Relative paths in the editor table were hard to read; missing library name caused the row to be skipped on Save.

**Current behaviour:** Browse inserts an **absolute** path; Save writes **relative** paths into `.projects`. Both resolve on load.

---

### Documentation list empty after Project Editor Save

**Cause (fixed):** Project reload cleared the Documentation panel but did not call `loadDocuments()` when the library was re-selected programmatically.

**Workaround on old builds:** Click the **Project** library name again in the left tree.

---

### Layout double-click expands tree instead of opening KLayout

**Expected:** Double-click on the **layout view root** opens KLayout and does **not** toggle expand. Use the tree arrow to browse hierarchy.

See [KLayout integration](../setup/KLAYOUT_INTEGRATION.md).

---

### Wrong or missing cell in KLayout after opening layout root

LibMan picks a root cell by: group name (if present in file) → single top cell → first top cell alphabetically. See [Root cell selection](../setup/KLAYOUT_INTEGRATION.md#root-cell-selection).

---

### Xschem does not start from LibMan (Windows)

**Symptoms:** Double-click **schematic** does nothing or WSL error.

**Check:**
1. Tool Manager → schematic tool → path to `...\Xschem\integrations\open-xschem-wsl.bat`
2. **Name(s)** = `schematic` (not `schematic.core`)
3. WSL: `xschem --version` works; `build-core-tcl.sh` was run
4. **CommonDB** is a sibling of **Xschem**, or set `COMMONDB_ROOT` in WSL (see [Xschem integration](../setup/XSCHEM_INTEGRATION.md))

---

### Xschem window is only a small title bar (WSL)

Corrupted `~/.xschem/geometry` with `1x1+` entries. See [Xschem integration — Troubleshooting](../setup/XSCHEM_INTEGRATION.md#troubleshooting).

---

## 🟡 Qt/GUI Issues

### GUI doesn't appear correctly

**Solution**:
1. Check Qt5 installation
2. Verify platform plugin is available
3. Set environment: `set QT_QPA_PLATFORM=windows`
4. Try Debug mode instead of Release

**Time**: 10 minutes

---

### Import: converter not found

**Symptoms:** Log shows `Converter 'xschem_to_core' was not found next to LibMan`.

**Solution:**
1. Rebuild with CORE enabled (not `CONFIG+=no_core`).
2. Confirm converter `.exe` files sit next to `libman.exe` (CMake POST_BUILD or qmake `core_converter_deploy.pri`).
3. Or set `LIBMAN_CONVERTER_DIR` to the tools directory.

See [IMPORT.md](../setup/IMPORT.md).

---

### Import: project file not updated

**Symptoms:** Views appear in LibMan but `.projects` has no new `define()` lines.

**Solution:** Open the project first (**File → Open...**). Import auto-saves only when a project is loaded; otherwise **File → Save** after import.

---

## ✅ Getting Help

1. **Quick answers**: See [Quick Reference](QUICK_REFERENCE.md)
2. **Setup help**: See [VS Code Setup](../setup/VSCODE_SETUP.md)
3. **Project Editor**: See [PROJECT_EDITOR.md](../setup/PROJECT_EDITOR.md)
4. **Import**: See [IMPORT.md](../setup/IMPORT.md)
5. **Xschem / WSL**: See [XSCHEM_INTEGRATION.md](../setup/XSCHEM_INTEGRATION.md)
6. **Dependencies**: See [DEPENDENCIES.md](../getting-started/DEPENDENCIES.md)
7. **All docs**: See [Documentation Index](../INDEX.md)

---

## 🔧 Debug Checklist

When something fails:

1. ✅ Check error message carefully
2. ✅ See if it matches one above
3. ✅ Follow solution steps
4. ✅ Check if issue is resolved
5. ✅ Try solutions in order of time (quickest first)

---

## 📞 Still Not Working?

1. **Verify setup**: See [SETUP_VERIFICATION.md](../getting-started/SETUP_VERIFICATION.md)
2. **Check dependencies**: See [DEPENDENCIES.md](../getting-started/DEPENDENCIES.md)
3. **Review VS Code setup**: See [VSCODE_SETUP.md](../setup/VSCODE_SETUP.md)
4. **Look in error output**: Check terminal for detailed errors

---

**Back to**: [Documentation Index](../INDEX.md)
