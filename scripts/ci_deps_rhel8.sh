#!/usr/bin/env bash
# Build dependencies for RHEL 8 / Rocky Linux 8 / AlmaLinux 8 CI images.
set -euo pipefail

dnf install -y epel-release
dnf install -y \
    gcc gcc-c++ make git autoconf automake libtool pkgconfig \
    python3 python3-pip \
    curl tar gzip which file patch patchelf \
    mesa-libGL-devel zlib-devel \
    dbus-libs fontconfig freetype \
    libxcb libxcb-devel libX11-devel \
    libxkbcommon-x11 libxkbcommon-x11-devel \
    openssl-devel glib2-devel \
    libXi-devel libXrender-devel \
    xorg-x11-fonts-Type1
