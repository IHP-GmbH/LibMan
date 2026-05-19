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

if [ -f "$STAMP_FILE" ] && [ -x "$INSTALL_DIR/bin/capnp" ]; then
    echo "Cap'n Proto already installed at $INSTALL_DIR"
    exit 0
fi

if [ ! -d "$REPO_DIR/.git" ]; then
    if [ -e "$REPO_DIR" ]; then
        echo "ERROR: $REPO_DIR exists but is not a git clone."
        echo "Remove it so Cap'n Proto can be cloned automatically: rm -rf \"$REPO_DIR\""
        exit 1
    fi
    git clone "$GIT_URL" "$REPO_DIR"
fi

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

cd "$REPO_DIR/c++"
autoreconf -i
./configure --prefix="$INSTALL_DIR"
make -j"$(nproc)" check
make install

echo "$TARGET_REV" > "$STATE_FILE"
touch "$STAMP_FILE"

echo "Cap'n Proto installed at revision $TARGET_REV"

