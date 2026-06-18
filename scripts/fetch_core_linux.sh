#!/usr/bin/env bash
# Clone CommonDB (CORE) and build static libs for qmake builds.
set -euo pipefail

LIBMAN_ROOT="${1:?LibMan repository root required}"
CAPNP_ROOT="${LIBMAN_ROOT}/capnp-install"
CORE_BUILD="${LIBMAN_ROOT}/.deps/core-build"
STAMP="${CORE_BUILD}/libman_core_built.stamp"

CORE_GIT_URL="${CORE_GIT_URL:-https://github.com/IHP-GmbH/CommonDB.git}"
CORE_GIT_BRANCH="${CORE_GIT_BRANCH:-main}"

if [ -f "$STAMP" ]; then
    echo "CORE already built ($STAMP)"
    exit 0
fi

if [ -n "${LIBMAN_CORE_SOURCE_DIR:-}" ] && [ -d "${LIBMAN_CORE_SOURCE_DIR}" ]; then
    CORE_SRC="${LIBMAN_CORE_SOURCE_DIR}"
    echo "Using local CORE tree: ${CORE_SRC}"
else
    CORE_SRC="${LIBMAN_ROOT}/.deps/CommonDB"
    mkdir -p "$(dirname "$CORE_SRC")"
    if [ ! -d "$CORE_SRC/.git" ]; then
        echo "Cloning CORE from ${CORE_GIT_URL} (${CORE_GIT_BRANCH})..."
        git clone --depth 1 --branch "$CORE_GIT_BRANCH" "$CORE_GIT_URL" "$CORE_SRC"
    fi
fi

if [ ! -f "${CAPNP_ROOT}/include/capnp/message.h" ]; then
    echo "ERROR: Cap'n Proto not found in ${CAPNP_ROOT}. Run capnp_install first." >&2
    exit 1
fi

cmake -S "$CORE_SRC" -B "$CORE_BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCORE_BOOTSTRAP_CAPNP=OFF \
    -DCAPNP_ROOT="$CAPNP_ROOT" \
    -DCORE_BUILD_TESTS=OFF \
    -DCORE_BUILD_OAS_TESTS=OFF \
    -DCORE_BUILD_EXAMPLES=OFF

cmake --build "$CORE_BUILD" --target core core_utils -j"$(nproc 2>/dev/null || echo 2)"

touch "$STAMP"
echo "CORE built in ${CORE_BUILD}"
