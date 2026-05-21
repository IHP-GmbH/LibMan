#!/usr/bin/env bash

set -u

# =========================
# Config
# =========================
REPORT_NAME="coverage.html"
TEST_LOG="test_results.txt"
BIN_NAME="tst_libman_gui"
FOUND_EXE=""

# =========================
# Resolve paths
# =========================
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
TEST_BUILD_DIR="${TEST_BUILD_DIR:-$ROOT_DIR/build-tests}"
LEGACY_TEST_BUILD_DIR="$ROOT_DIR/tests/build"
TESTDATA_SRC="$ROOT_DIR/tests/data"
TEST_OBJECT_DIR=""
ROOT_FWD="$ROOT_DIR"

# =========================
# Find test executable
# =========================
find_exe() {
    local search_dir="$1"

    if [[ -d "$search_dir" ]]; then
        FOUND_EXE="$(find "$search_dir" -type f -name "$BIN_NAME" | head -n 1)"
    fi
}

for search_dir in "$TEST_BUILD_DIR" "$BUILD_DIR" "$LEGACY_TEST_BUILD_DIR"; do
    if [[ -n "$FOUND_EXE" ]]; then
        break
    fi
    find_exe "$search_dir"
done

if [[ -z "$FOUND_EXE" ]]; then
    echo "Error: $BIN_NAME not found."
    exit 1
fi

TEST_OBJECT_DIR="$(dirname "$FOUND_EXE")"

# =========================
# Sync fixtures next to the test binary
# =========================
# QFINDTESTDATA prefers applicationDirPath()/data. qmake TESTDATA only copies a
# subset; mirror the full tree so GUI tests find GDS/OAS/LStream fixtures in CI.
sync_test_fixtures() {
    if [[ ! -d "$TESTDATA_SRC" ]]; then
        echo "Warning: test data source not found: $TESTDATA_SRC"
        return 0
    fi

    local dst="$TEST_OBJECT_DIR/data"
    mkdir -p "$dst"
    echo "Syncing test fixtures to \"$dst\"..."
    cp -a "$TESTDATA_SRC/." "$dst/"
}

sync_test_fixtures

# =========================
# Clean coverage artifacts (object dir only)
# =========================
echo "Cleaning old coverage data in \"$TEST_OBJECT_DIR\"..."
find "$TEST_OBJECT_DIR" -type f \( -name "*.gcda" -o -name "*.gcov" \) -delete 2>/dev/null

# =========================
# Run tests
# =========================
echo "Running: \"$FOUND_EXE\""

rm -f "$ROOT_DIR/$TEST_LOG"

pushd "$ROOT_DIR" >/dev/null || exit 1
"$FOUND_EXE"
TEST_EXIT=$?
popd >/dev/null || exit 1

# =========================
# Show results
# =========================
if [[ -f "$ROOT_DIR/$TEST_LOG" ]]; then
    echo
    echo "==================================="
    echo "TEST RESULTS"
    echo "==================================="
    cat "$ROOT_DIR/$TEST_LOG"
    echo "==================================="
    echo
else
    echo "Warning: $TEST_LOG was not created."
fi

if [[ $TEST_EXIT -ne 0 ]]; then
    echo "Tests reported $TEST_EXIT failure(s) (see per-suite [FAIL] lines above)."
else
    echo "All tests passed."
fi

# =========================
# Coverage (gcovr)
# =========================
echo "Generating coverage report..."

pushd "$ROOT_DIR" >/dev/null || exit 1

echo "Using object directory: \"$TEST_OBJECT_DIR\""
echo "Using source root: \"$ROOT_FWD\""

python -m gcovr -j 1 \
  -r "$ROOT_FWD" \
  --object-directory "$TEST_OBJECT_DIR" \
  --gcov-ignore-errors=all \
  --filter "$ROOT_FWD/.*" \
  --exclude "$ROOT_FWD/tests/.*" \
  --exclude "$ROOT_FWD/build/.*" \
  --exclude "$ROOT_FWD/build-tests/.*" \
  --exclude "$ROOT_FWD/extension/.*" \
  --exclude "$ROOT_FWD/capnp/.*" \
  --exclude "$ROOT_FWD/QtPropertyBrowser/.*" \
  --exclude "$ROOT_FWD/capnp-install/.*" \
  --exclude "$ROOT_FWD/oas/oasReader.cpp" \
  --exclude "$ROOT_FWD/oas/oasCreate.cpp" \
  --exclude "$ROOT_FWD/oas/oasReadAsync.cpp" \
  --exclude "$ROOT_FWD/src/viewcontextmenu.cpp" \
  --exclude "$ROOT_FWD/src/groupcontextmenu.cpp" \
  --exclude "$ROOT_FWD/src/projectcontextmenu.cpp" \
  --exclude "$ROOT_FWD/src/categorycontextmenu.cpp" \
  --exclude "$ROOT_FWD/src/mainwindow.cpp" \
  --exclude "$ROOT_FWD/src/klayoutServer.cpp" \
  --exclude "$ROOT_FWD/src/projectmanager.cpp" \
  --exclude ".*moc_.*" \
  --exclude ".*qrc_.*" \
  --html-details -o "$REPORT_NAME" \
  --print-summary

GCOVR_EXIT=$?

if [[ -f "$REPORT_NAME" ]]; then
    if command -v xdg-open >/dev/null 2>&1; then
        xdg-open "$REPORT_NAME" >/dev/null 2>&1 &
    fi
    find "$ROOT_DIR" -type f -name "*.gcov" -delete 2>/dev/null
else
    echo "Error: $REPORT_NAME not generated."
fi

popd >/dev/null || exit 1

# =========================
# Final exit code
# =========================
echo
echo "Summary: test failures=$TEST_EXIT, gcovr exit=$GCOVR_EXIT"

if [[ $GCOVR_EXIT -ne 0 ]]; then
    echo "Error: gcovr failed."
    exit "$GCOVR_EXIT"
fi

if [[ $TEST_EXIT -ne 0 ]]; then
    echo "Error: one or more tests failed."
    exit 1
fi

exit 0
