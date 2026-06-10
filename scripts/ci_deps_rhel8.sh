#!/usr/bin/env bash
# Build dependencies for RHEL 8 / Rocky Linux 8 / AlmaLinux 8 CI images.
set -euo pipefail

dnf install -y epel-release dnf-plugins-core

# Qt headers for xkb/xcb may live in powertools/crb on EL8 derivatives.
dnf config-manager --set-enabled powertools 2>/dev/null || \
dnf config-manager --set-enabled crb 2>/dev/null || true

dnf install -y \
    gcc gcc-c++ make git autoconf automake libtool pkgconfig \
    python3 python3-pip p7zip p7zip-plugins \
    curl tar gzip which file patch patchelf \
    mesa-libGL-devel zlib-devel libdrm-devel \
    dbus-libs fontconfig-devel freetype-devel \
    libxcb libxcb-devel libX11-devel libXext-devel libXfixes-devel \
    xcb-util xcb-util-devel xcb-util-image-devel xcb-util-keysyms-devel \
    xcb-util-renderutil-devel xcb-util-wm-devel \
    libxkbcommon libxkbcommon-devel \
    openssl-devel glib2-devel libicu-devel nss-devel \
    libXi-devel libXrender-devel \
    xorg-x11-fonts-Type1
