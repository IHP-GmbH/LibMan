#!/usr/bin/env bash
# Bundle Qt runtime libraries/plugins without linuxdeployqt (Ubuntu 24+ / glibc > 2.35).
set -euo pipefail

BINARY="${1:?binary path required}"
STAGE_DIR="${2:?stage directory required}"

if [[ ! -f "$BINARY" ]]; then
  echo "ERROR: binary not found: $BINARY" >&2
  exit 1
fi

QT_PREFIX="${QT_PREFIX:-$(qmake -query QT_INSTALL_PREFIX)}"
QT_LIBS="${QT_LIBS:-$(qmake -query QT_INSTALL_LIBS)}"
QT_PLUGINS="${QT_PLUGINS:-$(qmake -query QT_INSTALL_PLUGINS)}"

mkdir -p "${STAGE_DIR}/lib" "${STAGE_DIR}/plugins"

bundle_skip_system_lib() {
  case "$1" in
    /lib/*|/lib64/*|/usr/lib/*|/usr/lib64/*) return 0 ;;
  esac
  return 1
}

bundle_copy_lib() {
  local lib="$1"
  [[ -n "$lib" && -f "$lib" ]] || return 0
  bundle_skip_system_lib "$lib" && return 0
  cp -an "$lib" "${STAGE_DIR}/lib/" 2>/dev/null || cp -L "$lib" "${STAGE_DIR}/lib/" || true
}

bundle_copy_ldd() {
  local bin="$1"
  while IFS= read -r lib; do
    bundle_copy_lib "$lib"
  done < <(ldd "$bin" | awk '/=> \// {print $3}' || true)
}

bundle_copy_ldd "$BINARY"

if compgen -G "${QT_LIBS}/libQt"*.so* >/dev/null; then
  cp -a "${QT_LIBS}"/libQt*.so* "${STAGE_DIR}/lib/"
fi
cp -a "${QT_LIBS}"/libicu*.so* "${STAGE_DIR}/lib/" 2>/dev/null || true

for plugindir in platforms imageformats xcbglintegrations platforminputcontexts iconengines styles; do
  if [[ -d "${QT_PLUGINS}/${plugindir}" ]]; then
    cp -a "${QT_PLUGINS}/${plugindir}" "${STAGE_DIR}/plugins/"
  fi
done

while IFS= read -r -d '' plug; do
  while IFS= read -r lib; do
    [[ -n "${lib}" && -f "${lib}" ]] || continue
    case "${lib}" in
      "${QT_PREFIX}"/*|"${QT_LIBS}"/*)
        bundle_copy_lib "$lib"
        ;;
    esac
  done < <(ldd "$plug" | awk '/=>/{print $3}' || true)
done < <(find "${STAGE_DIR}/plugins" -type f -name '*.so' -print0 2>/dev/null || true)

echo "Manual Qt bundle staged under ${STAGE_DIR}"
