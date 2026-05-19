#!/usr/bin/env bash
set -e

GIT_URL="$1"
VERSION_MODE="$2"
GIT_BRANCH="$3"
GIT_TAG="$4"
GIT_COMMIT="$5"
REPO_DIR="$6"
INSTALL_DIR="$7"

STAMP_FILE="$INSTALL_DIR/.built"
STATE_FILE="$INSTALL_DIR/.capnp_revision"

mkdir -p "$(dirname "$REPO_DIR")"
mkdir -p "$INSTALL_DIR"

capnp_sources_present() {
    [ -d "$REPO_DIR/c++" ] && { [ -f "$REPO_DIR/c++/configure.ac" ] || [ -f "$REPO_DIR/c++/CMakeLists.txt" ]; }
}

if [ -f "$STAMP_FILE" ] && [ -x "$INSTALL_DIR/bin/capnp" ]; then
    echo "Cap'n Proto already installed at $INSTALL_DIR"
    exit 0
fi

USE_VENDORED=0
if [ -d "$REPO_DIR/.git" ]; then
    :
elif capnp_sources_present; then
    USE_VENDORED=1
    echo "Using vendored Cap'n Proto sources at $REPO_DIR"
elif [ -e "$REPO_DIR" ]; then
    echo "ERROR: $REPO_DIR exists but is not a git clone and does not contain Cap'n Proto sources"
    exit 1
else
    git clone "$GIT_URL" "$REPO_DIR"
fi

if [ "$USE_VENDORED" -eq 0 ]; then
    cd "$REPO_DIR"
    git fetch --tags origin

    case "$VERSION_MODE" in
      branch)
        TARGET_REF="origin/$GIT_BRANCH"
        TARGET_REV="$(git rev-parse "$TARGET_REF")"
        ;;
      tag)
        TARGET_REF="refs/tags/$GIT_TAG"
        TARGET_REV="$(git rev-parse "$TARGET_REF")"
        ;;
      commit)
        TARGET_REF="$GIT_COMMIT"
        TARGET_REV="$(git rev-parse "$TARGET_REF")"
        ;;
      *)
        echo "Unsupported CAPNP_VERSION_MODE: $VERSION_MODE"
        exit 1
        ;;
    esac

    CURRENT_REV=""
    if [ -f "$STATE_FILE" ]; then
        CURRENT_REV="$(cat "$STATE_FILE")"
    fi

    if [ "$CURRENT_REV" = "$TARGET_REV" ] && [ -f "$STAMP_FILE" ]; then
        echo "Cap'n Proto is already up to date: $TARGET_REV"
        exit 0
    fi

    git reset --hard "$TARGET_REV"
else
    TARGET_REV="vendored"
fi

cd "$REPO_DIR/c++"
autoreconf -i
./configure --prefix="$INSTALL_DIR"
if [ "$USE_VENDORED" -eq 1 ]; then
    make -j"$(nproc)"
else
    make -j"$(nproc)" check
fi
make install

echo "$TARGET_REV" > "$STATE_FILE"
touch "$STAMP_FILE"

echo "Cap'n Proto installed at revision $TARGET_REV"

