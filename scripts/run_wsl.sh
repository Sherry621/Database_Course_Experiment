#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}/.."

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

if [[ ! -x ./build/GenealogySystem ]]; then
  echo "Missing ./build/GenealogySystem. Run: cmake -S . -B build -G Ninja && cmake --build build" >&2
  exit 1
fi

exec ./build/GenealogySystem
