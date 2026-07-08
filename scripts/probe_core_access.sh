#!/usr/bin/env bash
# Exit 0 if CommonDB (CORE) is available for this build, 1 otherwise.
set -euo pipefail

LIBMAN_ROOT="${1:?LibMan repository root required}"
CORE_REPO="${CORE_GITHUB_REPO:-IHP-GmbH/CommonDB}"

if [ -n "${LIBMAN_CORE_SOURCE_DIR:-}" ] && [ -f "${LIBMAN_CORE_SOURCE_DIR}/src/core_paths.h" ]; then
    exit 0
fi

if [ -f "${LIBMAN_ROOT}/.deps/CommonDB/src/core_paths.h" ]; then
    exit 0
fi

token="${LIBMAN_CORE_GIT_TOKEN:-${GITHUB_TOKEN:-}}"
api_url="https://api.github.com/repos/${CORE_REPO}"

curl_args=(--connect-timeout 5 --max-time 10 -sS -o /dev/null -w "%{http_code}")
if [ -n "$token" ]; then
    curl_args+=(-H "Authorization: Bearer ${token}")
fi

http_code="$(curl "${curl_args[@]}" "$api_url" 2>/dev/null || echo 000)"
if [ "$http_code" = "200" ]; then
    exit 0
fi

exit 1
