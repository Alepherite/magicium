#!/bin/bash
set -e

# Base directories
LOCAL_BIN="$HOME/.local/bin"
LOCAL_SHARE="$HOME/.local/share/magicium"
APP_DIR="$HOME/.local/share/applications"

echo "========================================="
echo "       Magicium Converter Installer      "
echo "========================================="

# 1. Compile the project using Makefile
echo "Compiling the project..."
make

# 2. Check if build succeeded
if [ ! -f "build/magicium" ]; then
    echo "Error: Build failed. 'build/magicium' not found."
    exit 1
fi

# 3. Create structural directories if they do not exist
mkdir -p "$LOCAL_BIN"
mkdir -p "$LOCAL_SHARE"
mkdir -p "$APP_DIR"

# 4. Copy binary and UI assets to standard user paths
echo "Installing binaries and UI templates..."
cp build/magicium "$LOCAL_BIN/magicium"
chmod +x "$LOCAL_BIN/magicium"

# Copy UI directory
cp -r ui "$LOCAL_SHARE/"

# Định nghĩa thư mục lưu trữ icon chuẩn cho user
LOCAL_ICONS="$HOME/.local/share/icons/hicolor/128x128/apps"
mkdir -p "$LOCAL_ICONS"

# Giả sử bạn có file icon tên là 'icon.png' nằm trong thư mục assets/
if [ -f "assets/icon.png" ]; then
    echo "Installing application icon..."
    cp assets/icon.png "$LOCAL_ICONS/magicium-converter.png"
    
    # Lệnh bắt buộc để hệ thống (Sway/Rofi) nạp lại danh sách icon mới ngay lập tức
    gtk-update-icon-cache -f -t "$HOME/.local/share/icons/hicolor" 2>/dev/null || true
else
    echo "Warning: assets/icon.png not found. Skipping icon installation."
fi

echo "Core installation completed successfully."
echo "========================================="

# 5. Prompt for .desktop launcher installation
while true; do
    read -p "Do you want to install the desktop launcher shortcut? (y/n): " yn
    case $yn in
        [Yy]* )
            if [ -f "assets/magicium.desktop" ]; then
                cp assets/magicium.desktop "$APP_DIR/"
                chmod +x "$APP_DIR/magicium.desktop"
                echo "Desktop shortcut installed to $APP_DIR/magicium.desktop"
            else
                echo "Warning: assets/magicium.desktop not found. Skipping shortcut registration."
            fi
            break;
            ;;
        [Nn]* )
            echo "Skipping desktop shortcut installation."
            break;
            ;;
        * )
            echo "Please answer with (y)es or (n)o."
            ;;
    esac
done

echo "========================================="
echo "Installation finished! You can now run 'magicium' from your terminal or launcher."