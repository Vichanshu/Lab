#!/usr/bin/env bash
# Hide the Wi-Fi icon in the GNOME top bar while leaving the real connection up.
#
# Copy this one file to another machine and run it there. It installs a
# user extension and picks the GNOME 42-44 or GNOME 45+ format for that
# machine. It does not use root and does not change NetworkManager.
#
# The extension hides the status icon and draws the Quick Settings Wi-Fi
# switch as off. It uses a slow timer and does not listen to Wi-Fi updates,
# so switching networks in Settings cannot lock the desktop.
#
# Usage:
#   ./wifi-looks-off.sh on
#   ./wifi-looks-off.sh off
#   ./wifi-looks-off.sh uninstall
#   ./wifi-looks-off.sh status

set -euo pipefail

UUID="wifi-looks-off@local"
DISK_VERSION=4
EXT_ROOT="${HOME}/.local/share/gnome-shell/extensions"
DEST="${EXT_ROOT}/${UUID}"

usage() {
    cat <<'EOF'
Usage: wifi-looks-off.sh <on|off|uninstall|status>

  on         Hide the top-bar Wi-Fi icon. The connection stays up.
  off        Show the normal Wi-Fi icon again.
  uninstall  Show the normal icon and delete only this extension.
  status     Show whether the effect is on, and the real radio state.
EOF
}

refuse_root() {
    if [[ "${EUID:-$(id -u)}" -eq 0 ]]; then
        echo "Run this as your normal desktop user, without sudo." >&2
        exit 1
    fi
}

check_dest() {
    case "$DEST" in
        "${HOME}/.local/share/gnome-shell/extensions/${UUID}") ;;
        *)
            echo "Refusing to touch an unexpected path: ${DEST}" >&2
            exit 1
            ;;
    esac
    if [[ -L "$DEST" ]]; then
        echo "Refusing to follow a symlink at ${DEST}" >&2
        exit 1
    fi
}

need_gnome_extensions() {
    if ! command -v gnome-extensions >/dev/null 2>&1; then
        echo "gnome-extensions is not available. No changes made." >&2
        exit 1
    fi
    if ! command -v python3 >/dev/null 2>&1; then
        echo "python3 is not available. No changes made." >&2
        exit 1
    fi
}

extension_active() {
    gnome-extensions info "$UUID" 2>/dev/null | grep -q "State: ACTIVE"
}

shell_extension_version() {
    gnome-extensions info "$UUID" 2>/dev/null | awk '/^  Version:/ { print $2; exit }'
}

gnome_major() {
    local line major
    line="$(gnome-shell --version 2>/dev/null || true)"
    major="$(printf '%s\n' "$line" | sed -n 's/.* \([0-9][0-9]*\)\..*/\1/p')"
    if [[ ! "$major" =~ ^[0-9]+$ ]]; then
        echo "Could not read the GNOME Shell version. No changes made." >&2
        exit 1
    fi
    if (( major < 42 )); then
        echo "GNOME Shell ${major} is older than this script supports. No changes made." >&2
        exit 1
    fi
    printf '%s\n' "$major"
}

require_gnome_session() {
    local desktop="${XDG_CURRENT_DESKTOP:-}"
    case "$desktop" in
        *Budgie*|*Cinnamon*|*XFCE*|*KDE*|*MATE*|*LXQt*|*LXDE*)
            echo "This session is ${desktop}. The script only changes the GNOME top bar. No changes made." >&2
            exit 1
            ;;
        *GNOME*) ;;
        *)
            echo "Run this inside a GNOME session. No changes made." >&2
            exit 1
            ;;
    esac
}

# Edit one GNOME string-array setting, changing only this extension's uuid.
gsettings_array() {
    local action="$1"
    local key="$2"
    python3 - "$action" "$key" "$UUID" <<'PY'
import ast, subprocess, sys
action, key, uuid = sys.argv[1:]
if action not in {"add", "remove"}:
    sys.exit("unsupported action")
if not uuid or any(c not in "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.@-" for c in uuid):
    sys.exit("refusing to write an unexpected extension id")
raw = subprocess.check_output(
    ["gsettings", "get", "org.gnome.shell", key], text=True).strip()
if raw.startswith("@as "):
    raw = raw[4:].strip()
values = ast.literal_eval(raw)
if not isinstance(values, list) or not all(isinstance(v, str) for v in values):
    sys.exit(f"refusing to rewrite {key}: unexpected value")
if action == "add":
    if uuid not in values:
        values.append(uuid)
else:
    values = [v for v in values if v != uuid]
encoded = "[" + ", ".join("'" + v + "'" for v in values) + "]"
subprocess.check_call(["gsettings", "set", "org.gnome.shell", key, encoded])
PY
}

uuid_in_key() {
    local key="$1"
    python3 - "$key" "$UUID" <<'PY'
import ast, subprocess, sys
key, uuid = sys.argv[1:]
raw = subprocess.check_output(
    ["gsettings", "get", "org.gnome.shell", key], text=True).strip()
if raw.startswith("@as "):
    raw = raw[4:].strip()
values = ast.literal_eval(raw)
sys.exit(0 if isinstance(values, list) and uuid in values else 1)
PY
}

metadata_is_ours() {
    [[ -f "${DEST}/metadata.json" ]] || return 1
    python3 - "$DEST/metadata.json" "$UUID" <<'PY'
import json, sys
data = json.load(open(sys.argv[1], encoding="utf-8"))
sys.exit(0 if data.get("uuid") == sys.argv[2] else 1)
PY
}

write_metadata() {
    local stage="$1"
    local versions_json="$2"
    cat > "${stage}/metadata.json" <<EOF
{
  "uuid": "${UUID}",
  "name": "Wi-Fi Looks Off",
  "description": "Shows Wi-Fi as off in the GNOME top bar and Quick Settings. The real connection is left unchanged.",
  "shell-version": ${versions_json},
  "session-modes": ["user"],
  "version": ${DISK_VERSION}
}
EOF
}

write_modern_extension() {
    local stage="$1"
    cat > "${stage}/extension.js" <<'EOF'
import GLib from 'gi://GLib';
import {Extension} from 'resource:///org/gnome/shell/extensions/extension.js';
import * as Main from 'resource:///org/gnome/shell/ui/main.js';

const DISABLED_ICON = 'network-wireless-disabled-symbolic';

function findNetwork() {
    const status = Main.panel?.statusArea;
    if (!status)
        return null;
    return status.quickSettings?._network
        || status.aggregateMenu?._network
        || status.network
        || null;
}

export default class WifiLooksOffExtension extends Extension {
    enable() {
        this._timer = 0;
        this._net = null;
        this._icon = null;
        this._wifi = null;
        this._origActivate = null;
        this._activateSaved = false;
        this._timer = GLib.timeout_add(GLib.PRIORITY_DEFAULT, 500, () => {
            try {
                this._tick();
            } catch (error) {
                logError(error, 'wifi-looks-off');
            }
            return GLib.SOURCE_CONTINUE;
        });
        try {
            this._tick();
        } catch (error) {
            logError(error, 'wifi-looks-off');
        }
        log('wifi-looks-off: Wi-Fi is shown as off');
    }

    disable() {
        if (this._timer) {
            GLib.source_remove(this._timer);
            this._timer = 0;
        }
        const icon = this._icon;
        const net = this._net;
        const wifi = this._wifi;
        this._icon = null;
        this._net = null;
        this._wifi = null;
        if (wifi && this._origActivate)
            wifi.activate = this._origActivate;
        this._origActivate = null;
        this._activateSaved = false;
        try {
            if (wifi?._client)
                wifi.checked = !!wifi._client.wireless_enabled;
            const source = wifi?._itemBinding?.source;
            if (wifi)
                wifi.menu_enabled = !source?.is_hotspot;
            source?.notify?.('icon-name');
            source?.notify?.('name');
            if (icon)
                icon.visible = true;
            if (net && typeof net._updateIcon === 'function')
                net._updateIcon();
        } catch (error) {
            logError(error, 'wifi-looks-off');
        }
    }

    _tick() {
        const net = findNetwork();
        if (!net)
            return;
        this._net = net;
        this._paintQuickToggle(net._wirelessToggle);
        this._hideStatusIcon(net);
    }

    _paintQuickToggle(wifi) {
        if (!wifi)
            return;
        if (!this._activateSaved && typeof wifi.activate === 'function') {
            this._wifi = wifi;
            this._origActivate = wifi.activate.bind(wifi);
            wifi.activate = () => {};
            this._activateSaved = true;
        }
        if (wifi.checked)
            wifi.checked = false;
        if (wifi.icon_name !== DISABLED_ICON)
            wifi.icon_name = DISABLED_ICON;
        if (wifi.subtitle)
            wifi.subtitle = null;
        if (wifi.menu_enabled)
            wifi.menu_enabled = false;
    }

    _hideStatusIcon(net) {
        const icon = net?._primaryIndicator;
        if (!icon)
            return;
        this._icon = icon;
        const name = icon.icon_name || '';
        if (name.startsWith('network-wireless') && icon.visible)
            icon.visible = false;
    }
}
EOF
}

write_legacy_extension() {
    local stage="$1"
    cat > "${stage}/extension.js" <<'EOF'
const GLib = imports.gi.GLib;
const Main = imports.ui.main;

const DISABLED_ICON = 'network-wireless-disabled-symbolic';

function findNetwork() {
    if (!Main.panel || !Main.panel.statusArea)
        return null;
    let status = Main.panel.statusArea;
    if (status.quickSettings && status.quickSettings._network)
        return status.quickSettings._network;
    if (status.aggregateMenu && status.aggregateMenu._network)
        return status.aggregateMenu._network;
    if (status.network)
        return status.network;
    return null;
}

var WifiLooksOffExtension = class WifiLooksOffExtension {
    enable() {
        this._timer = 0;
        this._net = null;
        this._icon = null;
        this._wifi = null;
        this._origActivate = null;
        this._activateSaved = false;
        this._timer = GLib.timeout_add(GLib.PRIORITY_DEFAULT, 500, () => {
            try {
                this._tick();
            } catch (error) {
                logError(error, 'wifi-looks-off');
            }
            return GLib.SOURCE_CONTINUE;
        });
        try {
            this._tick();
        } catch (error) {
            logError(error, 'wifi-looks-off');
        }
        log('wifi-looks-off: Wi-Fi is shown as off');
    }

    disable() {
        if (this._timer) {
            GLib.source_remove(this._timer);
            this._timer = 0;
        }
        let icon = this._icon;
        let net = this._net;
        let wifi = this._wifi;
        this._icon = null;
        this._net = null;
        this._wifi = null;
        if (wifi && this._origActivate)
            wifi.activate = this._origActivate;
        this._origActivate = null;
        this._activateSaved = false;
        try {
            if (wifi && wifi._client)
                wifi.checked = !!wifi._client.wireless_enabled;
            let source = wifi && wifi._itemBinding ? wifi._itemBinding.source : null;
            if (wifi)
                wifi.menu_enabled = !(source && source.is_hotspot);
            if (source) {
                source.notify('icon-name');
                source.notify('name');
            }
            if (icon)
                icon.visible = true;
            if (net && typeof net._updateIcon === 'function')
                net._updateIcon();
        } catch (error) {
            logError(error, 'wifi-looks-off');
        }
    }

    _tick() {
        let net = findNetwork();
        if (!net)
            return;
        this._net = net;
        this._paintQuickToggle(net._wirelessToggle);
        this._hideStatusIcon(net);
    }

    _paintQuickToggle(wifi) {
        if (!wifi)
            return;
        if (!this._activateSaved && typeof wifi.activate === 'function') {
            this._wifi = wifi;
            this._origActivate = wifi.activate.bind(wifi);
            wifi.activate = () => {};
            this._activateSaved = true;
        }
        if (wifi.checked)
            wifi.checked = false;
        if (wifi.icon_name !== DISABLED_ICON)
            wifi.icon_name = DISABLED_ICON;
        if (wifi.subtitle)
            wifi.subtitle = null;
        if (wifi.menu_enabled)
            wifi.menu_enabled = false;
    }

    _hideStatusIcon(net) {
        if (!net)
            return;
        let icon = net._primaryIndicator;
        if (!icon)
            return;
        this._icon = icon;
        let name = icon.icon_name || '';
        if (name.indexOf('network-wireless') === 0 && icon.visible)
            icon.visible = false;
    }
};

function init() {
    return new WifiLooksOffExtension();
}
EOF
}

install_extension() {
    local major="$1"
    check_dest
    local stage versions_json
    stage="$(mktemp -d)"
    trap 'rm -rf "$stage"' RETURN

    if (( major >= 45 )); then
        local -a versions=()
        local v end="$major"
        if (( end < 50 )); then
            end=50
        fi
        for (( v = 45; v <= end; v++ )); do
            versions+=("$v")
        done
        versions_json="["
        local first=1 item
        for item in "${versions[@]}"; do
            if (( first )); then
                versions_json+="\"${item}\""
                first=0
            else
                versions_json+=", \"${item}\""
            fi
        done
        versions_json+="]"
        write_metadata "$stage" "$versions_json"
        write_modern_extension "$stage"
    else
        write_metadata "$stage" '["42", "43", "44"]'
        write_legacy_extension "$stage"
    fi

    python3 -m json.tool "${stage}/metadata.json" >/dev/null
    mkdir -p "$EXT_ROOT"
    mkdir -p "$DEST"
    cp -f "${stage}/metadata.json" "${DEST}/metadata.json"
    cp -f "${stage}/extension.js" "${DEST}/extension.js"
    chmod 644 "${DEST}/metadata.json" "${DEST}/extension.js"
    rm -rf "$stage"
    trap - RETURN
}

cmd_on() {
    refuse_root
    need_gnome_extensions
    check_dest
    require_gnome_session

    local major loaded
    major="$(gnome_major)"

    if [[ "$(gsettings get org.gnome.shell disable-user-extensions 2>/dev/null || echo false)" == "true" ]]; then
        echo "GNOME is set to ignore user extensions. No changes made." >&2
        echo "Allow user extensions in the Extensions app, then run: $0 on" >&2
        exit 1
    fi

    loaded="$(shell_extension_version || true)"
    install_extension "$major"

    if [[ -n "$loaded" && "$loaded" != "$DISK_VERSION" ]]; then
        echo "Quick Settings will show Wi-Fi as off after the next login."
        echo "This login keeps hiding the top-bar icon until then."
        echo "Log out and back in. The effect turns on by itself."
        exit 0
    fi

    gsettings_array remove disabled-extensions
    gsettings_array add enabled-extensions

    if ! gnome-extensions enable "$UUID" >/dev/null 2>&1; then
        echo "Extension installed for GNOME Shell ${major}. It turns on at the next login."
        echo "Log out and back in when you want the top bar to change. This script will not log you out."
    elif extension_active; then
        echo "The top-bar Wi-Fi icon is hidden, and Quick Settings shows Wi-Fi as off."
    else
        echo "Extension installed for GNOME Shell ${major}. It turns on at the next login."
        echo "Log out and back in when you want the top bar to change. This script will not log you out."
    fi
    echo "The connection stays up. Switch networks in Settings."
    echo "Restore the normal icon with: $0 off"
    echo "Remove the extension with: $0 uninstall"
}

cmd_off() {
    refuse_root
    need_gnome_extensions
    if gnome-extensions info "$UUID" >/dev/null 2>&1; then
        gnome-extensions disable "$UUID" || true
    fi
    gsettings_array remove enabled-extensions
    echo "Normal Wi-Fi icon restored."
    if extension_active; then
        echo "The top bar still shows the effect. Log out and back in to finish restoring it."
    fi
}

cmd_uninstall() {
    refuse_root
    need_gnome_extensions
    check_dest
    if gnome-extensions info "$UUID" >/dev/null 2>&1; then
        gnome-extensions disable "$UUID" || true
    fi
    gsettings_array remove enabled-extensions
    if [[ -d "$DEST" ]]; then
        if metadata_is_ours; then
            rm -rf "$DEST"
            echo "Extension removed and the normal Wi-Fi icon restored."
        else
            echo "Left ${DEST} in place because it is not this extension." >&2
            exit 1
        fi
    else
        echo "Extension is already absent. Normal Wi-Fi icon restored."
    fi
}

cmd_status() {
    need_gnome_extensions
    if extension_active; then
        echo "effect: on"
    elif uuid_in_key enabled-extensions; then
        echo "effect: scheduled for the next login"
    elif [[ -d "$DEST" ]]; then
        echo "effect: installed, currently off"
    else
        echo "effect: not installed"
    fi
    if command -v nmcli >/dev/null 2>&1; then
        echo "radio: $(nmcli -t -f WIFI radio 2>/dev/null || echo unknown)"
        nmcli -t -f TYPE,STATE device status 2>/dev/null | awk -F: '$1=="wifi" { print "device: " $2 }'
    fi
}

main() {
    case "${1:-}" in
        on) cmd_on ;;
        off) cmd_off ;;
        uninstall) cmd_uninstall ;;
        status) cmd_status ;;
        *) usage; exit 1 ;;
    esac
}

main "$@"
