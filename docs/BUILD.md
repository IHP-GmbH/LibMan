# Building LibMan

LibMan has two supported build entry points: **qmake** (used in GitHub Actions) and **CMake** (used by VS Code tasks and local IDE workflows). Both fetch **Cap'n Proto** and **LStream schemas** on first build; they are not vendored in git.

---

## Generated directories (gitignored)

| Path | Purpose |
|------|---------|
| `capnproto/` | Cap'n Proto source (cloned from GitHub) |
| `capnp-install/` | Cap'n Proto install prefix (`bin/`, `include/`, `lib/`) |
| `capnp-install/capnp-built.stamp` | Stamp file when install succeeded |
| `capnp/` | Generated Cap'n Proto / LStream schema C++ sources |
| `capnp/.schemas_built` | Stamp when LStream schemas were generated |
| `.deps/lstream/` | LStream schema repository (cloned from Codeberg) |

---

## Cap'n Proto and LStream (all platforms)

- **Cap'n Proto** is cloned from `https://github.com/capnproto/capnproto.git` (branch `master` by default).
- **Linux/macOS**: built with autotools (`scripts/build_capnp_linux.sh`).
- **Windows**: built with CMake + MinGW (`scripts/build_capnp_windows.bat`, invoked via `scripts/mkcapnp.cmd`).
- **LStream schemas**: `scripts/update_lstream_schemas_linux.sh` or `scripts/mklstream.cmd` (Windows).

Set `CAPNP_SKIP_CHECK=1` to skip Cap'n Proto's test suite during install (CI uses this).

---

## qmake build (CI and Qt Creator)

### Linux / macOS

```bash
mkdir -p build && cd build
qmake ../libman.pro
make -j1 ../capnp-install/capnp-built.stamp   # Cap'n Proto first (serial)
make -j"$(nproc)"                             # application (parallel)
```

### Windows (MinGW)

**Qt Creator** uses a shadow build directory such as `build/Desktop_Qt_5_15_2_MinGW_64_bit-Debug/`. The qmake rules use absolute paths to `scripts/mkcapnp.cmd` and compute stamp paths relative to that directory automatically. Re-run **qmake** after pulling changes if Cap'n Proto recipes fail.

From the repository root, install Cap'n Proto **before** the parallel app build (CI does this explicitly):

```cmd
call scripts\mkcapnp.cmd
```

Then configure and patch the Makefile (order-only deps for Cap'n Proto consumers):

```bash
mkdir -p build && cd build
qmake CONFIG+=release ../libman.pro
bash ../scripts/patch_capnp_makefile.sh
mingw32-make -j"$(nproc)"
```

`patch_capnp_makefile.sh` is required on Windows because MinGW qmake does not reliably add stamp prerequisites to object rules; without it, parallel `mingw32-make` can compile before headers exist.

### Tests (qmake)

```bash
mkdir -p build-tests && cd build-tests
qmake CONFIG+=debug CONFIG+=coverage ../tests/tests.pro
make -j1 ../capnp-install/capnp-built.stamp
make -j"$(nproc)"
```

Linux CI runs tests via `scripts/run_tests.sh` with `LIBMAN_TEST_DATA_DIR` pointing at `tests/data`.

---

## CMake build (VS Code / local)

```bash
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j4
```

`CMakeLists.txt` runs `scripts/mkcapnp.cmd` (Windows) or `scripts/build_capnp_linux.sh` (Linux) automatically via custom commands that produce `capnp-install/capnp-built.stamp`, then updates LStream schemas via `scripts/mklstream.cmd` (Windows) or `scripts/update_lstream_schemas_linux.sh` (Linux).

See [Quick Start](getting-started/QUICK_START.md) and [Dependencies](getting-started/DEPENDENCIES.md) for ZLIB and Qt paths.

---

## GitHub Actions (`.github/workflows/build.yml`)

| Job | Build system | Cap'n Proto |
|-----|--------------|-------------|
| `build-linux` | qmake + make | `make -j1 ../capnp-install/capnp-built.stamp` then parallel make |
| `build-windows` | qmake + mingw32-make | `mkcapnp.cmd` before qmake; `patch_capnp_makefile.sh` after qmake |
| `tests-linux` | qmake tests + `run_tests.sh` | Same stamp step in `build-tests/` |

---

## Scripts reference

| Script | Role |
|--------|------|
| `scripts/mkcapnp.cmd` | Windows wrapper → `build_capnp_windows.bat` |
| `scripts/build_capnp_windows.bat` | Clone/build/install Cap'n Proto (CMake/MinGW) |
| `scripts/build_capnp_linux.sh` | Clone/build/install Cap'n Proto (autotools) |
| `scripts/mklstream.cmd` | Windows LStream schema update |
| `scripts/update_lstream_schemas_linux.sh` | Linux LStream schema update |
| `scripts/patch_capnp_makefile.sh` | Post-qmake Makefile patch (Windows only) |
| `capnp_deps.pri` / `capnp_deps_finalize.pri` | qmake Cap'n Proto + schema rules |

---

## Troubleshooting

- **`capnp/message.h: No such file or directory` (Windows)**  
  Cap'n Proto was not installed before compiling. Run `scripts\mkcapnp.cmd`, or use the two-step make + `patch_capnp_makefile.sh` flow above.

- **`No rule to make target '../capnp-install/.built'`**  
  Use `capnp-built.stamp`, not `.built` (renamed for qmake/Makefile compatibility).

- **First build is slow**  
  Normal: cloning and compiling Cap'n Proto takes several minutes.
