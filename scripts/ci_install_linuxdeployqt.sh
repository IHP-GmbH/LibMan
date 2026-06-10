#!/usr/bin/env bash
# Install linuxdeployqt into /usr/local/bin (CI helper).
set -euo pipefail

if command -v linuxdeployqt >/dev/null 2>&1; then
    linuxdeployqt -version
    exit 0
fi

URL="https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
APPIMAGE="/usr/local/bin/linuxdeployqt.AppImage"
WRAPPER="/usr/local/bin/linuxdeployqt"

curl -fsSL "$URL" -o "$APPIMAGE"
chmod +x "$APPIMAGE"

cat >"$WRAPPER" <<'EOF'
#!/usr/bin/env bash
exec /usr/local/bin/linuxdeployqt.AppImage --appimage-extract-and-run "$@"
EOF
chmod +x "$WRAPPER"

linuxdeployqt -version
