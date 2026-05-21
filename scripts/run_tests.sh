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
# Clean coverage artifacts
# =========================
echo "Cleaning old coverage data..."
find "$ROOT_DIR" -type f \( -name "*.gcda" -o -name "*.gcov" \) -delete 2>/dev/null

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

echo "Test exit code: $TEST_EXIT"
echo "Continuing with coverage generation..."

# =========================
# Coverage (gcovr)
# =========================
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
  --exclude ".*moc_.*" \
  --exclude ".*qrc_.*" \
  --html-details -o "$REPORT_NAME" \
  --print-summary

GCOVR_EXIT=$?

# =========================
# Open report
# =========================
if [[ -f "$REPORT_NAME" ]]; then
    xdg-open "$REPORT_NAME" >/dev/null 2>&1 &
    find "$ROOT_DIR" -type f -name "*.gcov" -delete 2>/dev/null
else
    echo "Error: $REPORT_NAME not generated."
fi

popd >/dev/null || exit 1

# =========================
# Final exit code
# =========================
if [[ $GCOVR_EXIT -ne 0 ]]; then
    exit "$GCOVR_EXIT"
fi

exit "$TEST_EXIT"