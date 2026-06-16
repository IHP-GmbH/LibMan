# CORE (CommonDB) integration

LibMan automatically downloads and builds [CORE](https://github.com/IHP-GmbH/CommonDB) during the first CMake configure. CORE shares LibMan's Cap'n Proto prefix (`capnp-install/`).

## Default behaviour

On `cmake -B build`:

1. Cap'n Proto is bootstrapped into `capnp-install/` if missing (same as LStream schemas).
2. CORE is cloned to `.deps/CommonDB/` from GitHub (`main` by default).
3. Static libraries `CORE::core` and `CORE::core_utils` are built and linked into `libman`.

Re-configure after changing the CORE revision:

```powershell
Remove-Item -Recurse -Force .deps\CommonDB
cmake -B build
```

Or pin a tag/commit:

```powershell
cmake -B build -DCORE_GIT_TAG=91705d7
```

## Local CORE checkout (development)

When working on CORE and LibMan side by side, skip the git fetch:

```powershell
cmake -B build -DLIBMAN_CORE_SOURCE_DIR=C:/Users/anton/Documents/CommonDB
```

## Installed CORE (advanced)

Disable automatic fetch and use a pre-installed package:

```powershell
cmake -B build -DLIBMAN_FETCH_CORE=OFF -DCORE_DIR=...
```

(`find_package(CORE)` — requires CORE installed with `cmake --install`.)

## CMake cache variables

| Variable | Default | Description |
|----------|---------|-------------|
| `LIBMAN_FETCH_CORE` | `ON` | Fetch CORE from GitHub |
| `CORE_GIT_URL` | `https://github.com/IHP-GmbH/CommonDB.git` | Repository URL |
| `CORE_GIT_TAG` | `main` | Branch, tag, or commit |
| `LIBMAN_CORE_SOURCE_DIR` | *(empty)* | Local tree instead of fetch |

## Using CORE in LibMan code

```cpp
#include "database.h"

core::Database db;
db.loadFromFile("layout.core");
```

Link targets are already set in `CMakeLists.txt` (`CORE::core`, `CORE::core_utils`).
