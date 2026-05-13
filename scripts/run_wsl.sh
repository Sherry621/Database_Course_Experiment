#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}/.."

# WSLg defaults to Wayland, but Qt6 + fcitx5 is more reliable through XWayland/xcb.
export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-xcb}"
export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/mnt/wslg/runtime-dir}"
export QT_IM_MODULE=fcitx
export GTK_IM_MODULE=fcitx
export XMODIFIERS=@im=fcitx

if command -v fcitx5 >/dev/null 2>&1; then
  if ! pgrep -x fcitx5 >/dev/null 2>&1; then
    fcitx5 -d >/tmp/genealogy_fcitx5.log 2>&1 || true
    sleep 1
  fi
  if ! pgrep -x fcitx5 >/dev/null 2>&1; then
    echo "Warning: fcitx5 did not stay running, Chinese input may not work." >&2
    echo "Check /tmp/genealogy_fcitx5.log or run: fcitx5-diagnose" >&2
  fi
else
  echo "Warning: fcitx5 is not installed, Chinese input will not work." >&2
  echo "Install: sudo apt-get install -y fcitx5 fcitx5-chinese-addons fcitx5-frontend-qt6" >&2
fi

if [[ ! -x ./build/GenealogySystem ]]; then
  echo "Missing ./build/GenealogySystem. Run: cmake -S . -B build -G Ninja && cmake --build build" >&2
  exit 1
fi

exec ./build/GenealogySystem
