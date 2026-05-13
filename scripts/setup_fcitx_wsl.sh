#!/usr/bin/env bash
set -euo pipefail

missing_packages=()
command -v fcitx5 >/dev/null 2>&1 || missing_packages+=("fcitx5")

if ! dpkg -s fcitx5-chinese-addons >/dev/null 2>&1; then
  missing_packages+=("fcitx5-chinese-addons")
fi

if ! dpkg -s fcitx5-frontend-qt6 >/dev/null 2>&1; then
  missing_packages+=("fcitx5-frontend-qt6")
fi

if (( ${#missing_packages[@]} > 0 )); then
  echo "Missing Chinese input packages: ${missing_packages[*]}" >&2
  echo "Install them first:" >&2
  echo "sudo apt-get update && sudo apt-get install -y fcitx5 fcitx5-chinese-addons fcitx5-frontend-qt6" >&2
  exit 1
fi

mkdir -p "${HOME}/.config/fcitx5"

export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-xcb}"
export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/mnt/wslg/runtime-dir}"
export QT_IM_MODULE=fcitx
export GTK_IM_MODULE=fcitx
export XMODIFIERS=@im=fcitx

pkill -x fcitx5 >/dev/null 2>&1 || true
sleep 1

cat > "${HOME}/.config/fcitx5/profile" <<'EOF'
[Groups/0]
# Group Name
Name=Default
# Layout
Default Layout=us
# Default Input Method
DefaultIM=pinyin

[Groups/0/Items/0]
# Name
Name=keyboard-us
# Layout
Layout=

[Groups/0/Items/1]
# Name
Name=pinyin
# Layout
Layout=

[GroupOrder]
0=Default
EOF

cat > "${HOME}/.xprofile" <<'EOF'
export QT_QPA_PLATFORM=xcb
export XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR:-/mnt/wslg/runtime-dir}
export QT_IM_MODULE=fcitx
export GTK_IM_MODULE=fcitx
export XMODIFIERS=@im=fcitx
EOF

fcitx5 -d >/tmp/genealogy_fcitx5.log 2>&1 || true
sleep 1

echo "fcitx5 has been configured with Pinyin."
if pgrep -x fcitx5 >/dev/null 2>&1; then
  echo "fcitx5 is running."
else
  echo "Warning: fcitx5 did not stay running." >&2
  echo "Check /tmp/genealogy_fcitx5.log or run: fcitx5-diagnose" >&2
fi
echo "Use ./scripts/run_wsl.sh from the project root to start the Qt app."
