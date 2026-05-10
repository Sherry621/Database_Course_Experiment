#!/usr/bin/env bash
set -euo pipefail

mkdir -p "${HOME}/.config/fcitx5"

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
export QT_IM_MODULE=fcitx
export GTK_IM_MODULE=fcitx
export XMODIFIERS=@im=fcitx
EOF

fcitx5 -d >/dev/null 2>&1 || true

echo "fcitx5 has been configured with Pinyin."
echo "Use ./run_wsl.sh to start the Qt app."
