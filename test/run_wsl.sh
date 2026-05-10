#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

export QT_IM_MODULE=fcitx
export GTK_IM_MODULE=fcitx
export XMODIFIERS=@im=fcitx

if command -v fcitx5 >/dev/null 2>&1; then
  if ! pgrep -x fcitx5 >/dev/null 2>&1; then
    fcitx5 -d >/dev/null 2>&1 || true
  fi
fi

# xcb usually works better with fcitx in WSLg/XWayland than native wayland.
export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-xcb}"
export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/mnt/wslg/runtime-dir}"

exec ./build-wsl/GenealogySystem
