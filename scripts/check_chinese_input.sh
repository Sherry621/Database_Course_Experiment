#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}/.."

export QT_QPA_PLATFORM=xcb
export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/mnt/wslg/runtime-dir}"
export QT_IM_MODULE=fcitx
export GTK_IM_MODULE=fcitx
export XMODIFIERS=@im=fcitx

echo "== Environment =="
echo "DISPLAY=${DISPLAY:-}"
echo "WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-}"
echo "XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR:-}"
echo "QT_QPA_PLATFORM=${QT_QPA_PLATFORM}"
echo "QT_IM_MODULE=${QT_IM_MODULE}"
echo "GTK_IM_MODULE=${GTK_IM_MODULE}"
echo "XMODIFIERS=${XMODIFIERS}"
echo

echo "== Packages =="
for pkg in fcitx5 fcitx5-chinese-addons fcitx5-frontend-qt6; do
  if dpkg -s "${pkg}" >/dev/null 2>&1; then
    echo "${pkg}: installed"
  else
    echo "${pkg}: missing"
  fi
done
echo

echo "== Fcitx process =="
if ! pgrep -x fcitx5 >/dev/null 2>&1; then
  echo "fcitx5 is not running, starting it..."
  fcitx5 -d >/tmp/genealogy_fcitx5.log 2>&1 || true
  sleep 1
fi
pgrep -a fcitx5 || true
echo

echo "== Fcitx profile =="
if [[ -f "${HOME}/.config/fcitx5/profile" ]]; then
  sed -n '1,120p' "${HOME}/.config/fcitx5/profile"
else
  echo "Missing ${HOME}/.config/fcitx5/profile. Run ./scripts/setup_fcitx_wsl.sh"
fi
echo

echo "== Qt6 input context probing =="
if command -v fcitx5-qt6-immodule-probing >/dev/null 2>&1; then
  fcitx5-qt6-immodule-probing
else
  echo "fcitx5-qt6-immodule-probing is missing. Install fcitx5-frontend-qt6."
fi
echo

echo "== Running GenealogySystem process =="
app_pid="$(pgrep -n GenealogySystem || true)"
if [[ -n "${app_pid}" ]]; then
  echo "PID=${app_pid}"
  echo "-- process command --"
  ps -p "${app_pid}" -o pid=,args=
  echo "-- selected process environment --"
  tr '\0' '\n' <"/proc/${app_pid}/environ" \
    | grep -E '^(QT_QPA_PLATFORM|QT_IM_MODULE|GTK_IM_MODULE|XMODIFIERS|DISPLAY|WAYLAND_DISPLAY|XDG_RUNTIME_DIR|DBUS_SESSION_BUS_ADDRESS)=' \
    | sort
else
  echo "GenealogySystem is not running."
  echo "Start it with ./scripts/run_wsl.sh, keep the window open, then run this script again."
fi
echo

echo "== Fcitx DBus state =="
if command -v dbus-send >/dev/null 2>&1; then
  dbus-send --session --print-reply --dest=org.fcitx.Fcitx5 \
    /controller org.fcitx.Fcitx.Controller1.DebugInfo 2>/dev/null \
    | sed -n '1,80p' || true
else
  echo "dbus-send is missing."
fi
echo

echo "== Expected result =="
echo "The Qt6 probing output should contain:"
echo "IM_MODULE_CLASSNAME=fcitx::QFcitxPlatformInputContext"
echo
echo "Then start the app with:"
echo "./scripts/run_wsl.sh"
echo
echo "In a Qt input field, press Ctrl+Space and type: ceshi"
