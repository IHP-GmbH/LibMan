#!/usr/bin/env bash
set -e

GIT_URL="$1"
VERSION_MODE="$2"
GIT_BRANCH="$3"
GIT_TAG="$4"
GIT_COMMIT="$5"
REPO_DIR="$6"
OUT_DIR="$7"
CAPNP_ROOT="$8"

STAMP_FILE="$OUT_DIR/.schemas_built"
STATE_FILE="$OUT_DIR/.lstream_revision"
CAPNP_EXE="$CAPNP_ROOT/bin/capnp"

if [ -z "$GIT_URL" ] || [ -z "$REPO_DIR" ] || [ -z "$OUT_DIR" ] || [ -z "$CAPNP_ROOT" ]; then
    echo "ERROR: missing required arguments"
    exit 1
fi

if [ ! -x "$CAPNP_EXE" ]; then
    echo "ERROR: capnp executable not found: $CAPNP_EXE"
    exit 1
fi

mkdir -p "$(dirname "$REPO_DIR")"

if [ ! -d "$REPO_DIR/.git" ]; then
    git clone "$GIT_URL" "$REPO_DIR"
fi

cd "$REPO_DIR"
git fetch --tags origin

case "$VERSION_MODE" in
  branch)
    TARGET_REV="$(git rev-parse "origin/$GIT_BRANCH")"
    ;;
  tag)
    TARGET_REV="$(git rev-parse "refs/tags/$GIT_TAG")"
    ;;
  commit)
    TARGET_REV="$(git rev-parse "$GIT_COMMIT")"
    ;;
  *)
    echo "ERROR: unsupported VERSION_MODE=$VERSION_MODE"
    exit 1
    ;;
esac

CURRENT_REV=""
if [ -f "$STATE_FILE" ]; then
    CURRENT_REV="$(cat "$STATE_FILE")"
fi

if [ "$CURRENT_REV" = "$TARGET_REV" ] && [ -f "$STAMP_FILE" ]; then
    echo "LStream schemas already up to date: $TARGET_REV"
    exit 0
fi

git reset --hard "$TARGET_REV"

mkdir -p "$OUT_DIR"
rm -f "$OUT_DIR"/*.capnp "$OUT_DIR"/*.capnp.h "$OUT_DIR"/*.capnp.c++ "$OUT_DIR"/*.capnp.cc

cp "$REPO_DIR"/capnp/*.capnp "$OUT_DIR"/

for f in "$OUT_DIR"/*.capnp; do
    "$CAPNP_EXE" compile -I "$OUT_DIR" -o c++ "$f"
done

for f in "$OUT_DIR"/*.capnp.c++; do
    [ -e "$f" ] || continue
    mv "$f" "${f%.c++}.cc"
done

echo "$TARGET_REV" > "$STATE_FILE"
touch "$STAMP_FILE"

echo "LStream schemas updated successfully."

