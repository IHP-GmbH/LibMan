#!/usr/bin/env bash
# Package libman + runtime libraries into a self-contained tar.gz bundle.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build}"
OUT_TAR="${2:-$ROOT/libman-linux-bundle.tar.gz}"
BUNDLE_LABEL="${3:-linux}"

DIST="$ROOT/dist-bundle"
STAGE="$ROOT/.bundle-stage"
rm -rf "$DIST" "$STAGE"
mkdir -p "$DIST/bin" "$DIST/lib" "$STAGE"

find_binary() {
    local candidate
    for candidate in \
        "$BUILD_DIR/libman" \
        "$BUILD_DIR/LibMan" \
        "$BUILD_DIR/release/libman" \
        "$BUILD_DIR/release/LibMan"; do
        if [[ -f "$candidate" ]]; then
            echo "$candidate"
            return 0
        fi
    done
    return 1
}

BIN="$(find_binary)" || {
    echo "ERROR: libman binary not found under $BUILD_DIR"
    ls -la "$BUILD_DIR" || true
    exit 1
}

cp "$BIN" "$STAGE/libman"
chmod +x "$STAGE/libman"

CAPNP_LIB="$ROOT/capnp-install/lib"
mkdir -p "$STAGE/lib"
if [[ -d "$CAPNP_LIB" ]]; then
    cp -a "$CAPNP_LIB"/lib*.so* "$STAGE/lib/" 2>/dev/null || true
fi

if [[ -n "${QT_ROOT_DIR:-}" && -d "${QT_ROOT_DIR}/bin" ]]; then
    export PATH="${QT_ROOT_DIR}/bin:${PATH}"
fi

DEPLOY_LDPATH="$STAGE/lib"
if [[ -n "${QT_ROOT_DIR:-}" && -d "${QT_ROOT_DIR}/lib" ]]; then
    DEPLOY_LDPATH="${DEPLOY_LDPATH}:${QT_ROOT_DIR}/lib"
fi
if [[ -n "${LD_LIBRARY_PATH:-}" ]]; then
    DEPLOY_LDPATH="${DEPLOY_LDPATH}:${LD_LIBRARY_PATH}"
fi
# linuxdeployqt runs ldd on Qt plugins (libqxcb.so); need host libs visible during packaging.
DEPLOY_LDPATH="${DEPLOY_LDPATH}:/usr/lib64:/lib64"
export LD_LIBRARY_PATH="$DEPLOY_LDPATH"

if command -v patchelf >/dev/null 2>&1; then
    patchelf --set-rpath '$ORIGIN/lib' "$STAGE/libman" || true
fi

bundle_manual() {
    bash "$(dirname "$0")/bundle_qt_libs_manual.sh" "$STAGE/libman" "$STAGE"
}

if [[ "${BUNDLE_SKIP_LINUXDEPLOYQT:-}" == "1" ]]; then
    bundle_manual
elif command -v linuxdeployqt >/dev/null 2>&1; then
    if ! linuxdeployqt "$STAGE/libman" -bundle-non-qt-libs -always-overwrite; then
        echo "linuxdeployqt failed; falling back to manual Qt bundling." >&2
        bundle_manual
    fi
else
    echo "WARNING: linuxdeployqt not found; using manual Qt bundling."
    bundle_manual
fi

cp "$STAGE/libman" "$DIST/bin/libman"
for so in "$STAGE"/*.so*; do
    [[ -e "$so" ]] || continue
    cp -a "$so" "$DIST/lib/"
done
if [[ -d "$STAGE/lib" ]]; then
    cp -a "$STAGE/lib/"* "$DIST/lib/" 2>/dev/null || true
fi
if [[ -d "$STAGE/plugins" ]]; then
    cp -a "$STAGE/plugins" "$DIST/"
fi

if [[ -d "$ROOT/capnp-install/lib" ]]; then
    cp -a "$ROOT/capnp-install/lib/"lib*.so* "$DIST/lib/" 2>/dev/null || true
fi

if command -v patchelf >/dev/null 2>&1; then
    patchelf --set-rpath '$ORIGIN/../lib' "$DIST/bin/libman" || true
fi

for dir in scripts keywords icons; do
    if [[ -d "$ROOT/$dir" ]]; then
        cp -a "$ROOT/$dir" "$DIST/"
    fi
done

cat >"$DIST/libman-run.sh" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="$DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="$DIR/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"
exec "$DIR/bin/libman" "$@"
EOF
chmod +x "$DIST/libman-run.sh"

cat >"$DIST/BUNDLE.txt" <<EOF
LibMan portable bundle ($BUNDLE_LABEL)
Built: $(date -u +%Y-%m-%dT%H:%M:%SZ)
Run: ./libman-run.sh [arguments]
Requires: glibc 2.28+ (RHEL 8.10 / Rocky Linux 8 / AlmaLinux 8 compatible)
EOF

rm -rf "$STAGE"
tar -C "$ROOT" -czf "$OUT_TAR" "$(basename "$DIST")"
echo "Created $OUT_TAR"
