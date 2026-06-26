#!/bin/bash
set -e

# Base directories
LOCAL_BIN="$HOME/.local/bin/magicium"
LOCAL_SHARE="$HOME/.local/share/magicium"
APP_DESKTOP="$HOME/.local/share/applications/magicium.desktop"
LOCAL_ICON="$HOME/.local/share/icons/hicolor/128x128/apps/magicium-converter.png"
CONFIG_DIR="$HOME/.config/magicium"
REPO_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "========================================="
echo "     Magicium Converter Uninstaller      "
echo "========================================="

# 1. Remove binary executable
if [ -f "$LOCAL_BIN" ]; then
    echo "Removing binary executable..."
    rm -f "$LOCAL_BIN"
fi

# 2. Remove shared resources (HTML/JS/CSS UI)
if [ -d "$LOCAL_SHARE" ]; then
    echo "Removing shared resources..."
    rm -rf "$LOCAL_SHARE"
fi

# 3. Remove desktop shortcut launcher
if [ -f "$APP_DESKTOP" ]; then
    echo "Removing desktop shortcut..."
    rm -f "$APP_DESKTOP"
fi

# 4. Remove system icon and update desktop icon cache
if [ -f "$LOCAL_ICON" ]; then
    echo "Removing application icon..."
    rm -f "$LOCAL_ICON"
    echo "Updating system icon cache..."
    gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" 2>/dev/null || true
fi

echo "Core system files uninstalled successfully."
echo "========================================="

# 5. Prompt for config directory removal (keeps user preferences optional)
while true; do
    read -p "Do you want to delete your configuration settings? (y/n): " yn
    case $yn in
        [Yy]* )
            if [ -d "$CONFIG_DIR" ]; then
                echo "Removing configuration directory..."
                rm -rf "$CONFIG_DIR"
            else
                echo "No configuration directory found."
            fi
            break;
            ;;
        [Nn]* )
            echo "Skipping configuration cleanup (keeping settings)."
            break;
            ;;
        * )
            echo "Please answer with (y)es or (n)o."
            ;;
    esac
done

echo "========================================="

# 6. Prompt to remove the cloned Git repository directory (self-deletion)
while true; do
    read -p "Do you want to completely remove this cloned repository directory? (y/n): " yn
    case $yn in
        [Yy]* )
            echo "Removing repository directory: $REPO_DIR"
            # Move shell out of active directory before deletion to prevent terminal path errors
            cd "$HOME"
            rm -rf "$REPO_DIR"
            echo "Repository directory removed."
            break;
            ;;
        [Nn]* )
            echo "Repository directory kept."
            break;
            ;;
        * )
            echo "Please answer with (y)es or (n)o."
            ;;
    esac
done

echo "========================================="
echo "Uninstallation finished cleanly!"