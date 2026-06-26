#!/bin/bash
set -e

########################################
# Detect execution mode
########################################

SCRIPT_PATH="$(realpath "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"

# If install.sh and Makefile exist beside this script,
# we're almost certainly inside the cloned repository.
if [[ -f "$SCRIPT_DIR/install.sh" && -f "$SCRIPT_DIR/Makefile" ]]; then
    REPO_MODE=true
else
    REPO_MODE=false
fi

########################################
# Installation paths
########################################

LOCAL_BIN="$HOME/.local/bin"
LOCAL_SHARE="$HOME/.local/share/magicium"
DESKTOP_ENTRY="$HOME/.local/share/applications/magicium.desktop"
ICON_FILE="$HOME/.local/share/icons/hicolor/128x128/apps/magicium-converter.png"
CONFIG_DIR="$HOME/.config/magicium"

APP_BINARY="$LOCAL_BIN/magicium"
GLOBAL_UNINSTALLER="$LOCAL_BIN/magicium-uninstall"

########################################

echo "========================================="
echo "        Magicium Uninstaller"
echo "========================================="

########################################
# Remove executable
########################################

if [[ -f "$APP_BINARY" ]]; then
    echo "Removing executable..."
    rm -f "$APP_BINARY"
fi

########################################
# Remove application files
########################################

if [[ -d "$LOCAL_SHARE" ]]; then
    echo "Removing application files..."
    rm -rf "$LOCAL_SHARE"
fi

########################################
# Remove desktop launcher
########################################

if [[ -f "$DESKTOP_ENTRY" ]]; then
    echo "Removing desktop entry..."
    rm -f "$DESKTOP_ENTRY"
fi

########################################
# Remove icon
########################################

if [[ -f "$ICON_FILE" ]]; then
    echo "Removing icon..."
    rm -f "$ICON_FILE"

    gtk-update-icon-cache -f -t \
        "$HOME/.local/share/icons/hicolor" \
        2>/dev/null || true
fi

########################################
# Ask about config
########################################

echo

while true; do
    read -rp "Remove your configuration (~/.config/magicium)? (y/n): " yn

    case "$yn" in
        [Yy]* )
            rm -rf "$CONFIG_DIR"
            echo "Configuration removed."
            break
            ;;

        [Nn]* )
            echo "Configuration kept."
            break
            ;;

        * )
            echo "Please answer y or n."
            ;;
    esac
done

########################################
# Offer deleting repository
########################################

if $REPO_MODE; then

    echo
    echo "Repository detected."

    while true; do
        read -rp "Delete this cloned repository as well? (y/n): " yn

        case "$yn" in
            [Yy]* )

                echo "Removing repository..."

                cd "$HOME"

                rm -rf "$SCRIPT_DIR"

                echo "Repository removed."

                break
                ;;

            [Nn]* )

                echo "Repository kept."

                break
                ;;

            * )
                echo "Please answer y or n."
                ;;
        esac
    done

fi

########################################
# Remove global uninstaller
########################################

if [[ "$SCRIPT_PATH" != "$GLOBAL_UNINSTALLER" ]]; then

    rm -f "$GLOBAL_UNINSTALLER"

else

    (
        sleep 0.5
        rm -f "$GLOBAL_UNINSTALLER"
    ) &

fi

########################################

echo
echo "Magicium has been successfully removed."
echo "========================================="