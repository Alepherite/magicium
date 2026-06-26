## Quick Start

Magicium is natively tailored for Linux desktop environments. Make sure you install the [Prerequisites](#prerequisites) first to ensure a smooth compilation.

### Option A: Automated Installation (Recommended)

Run the automated assistant to compile the source code and generate a desktop entry shortcut in your system launcher:

```bash
git clone https://github.com/Alepherite/magicium.git
cd magicium
chmod +x install.sh
./install.sh
```

### Option B: Manual Compilation

If you prefer building and launching the binary manually from your terminal:

```bash
git clone https://github.com/Alepherite/magicium.git
cd magicium
make
build/magicium
```

### Uninstall 

If you installed Magicium using the automated installer, you can completely remove it by running:

```bash
cd magicium
chmod +x uninstall.sh
./uninstall.sh
```

This removes the installed application, desktop entry, and related files created by the installer.

---

## Prerequisites

Install the required development libraries depending on your Linux distribution.

### Debian / Ubuntu / Linux Mint

```bash
sudo apt install build-essential libgtk-3-dev libwebkit2gtk-4.1-dev imagemagick libnotify-bin
```

### Fedora / RHEL

```bash
sudo dnf install gcc-c++ make gtk3-devel webkit2gtk4.1-devel ImageMagick libnotify
```

### Arch Linux / CachyOS / Manjaro

```bash
sudo pacman -S base-devel gtk3 webkit2gtk-4.1 imagemagick libnotify
```

---

## Features

- **Hybrid Architecture** — A modern, fluid HTML5/JS/CSS interface powered by WebKitGTK, sitting directly on top of a compiled C++ core.
- **Zero Base64 Overhead** — Bypasses slow browser-side canvas encoding by sending absolute physical file paths straight to the system's native `magick` binary.
- **Asynchronous Processing** — Offloads conversions to isolated, non-blocking background threads (`g_spawn_command_line_async`) to prevent UI freezing.
- **Desktop Drag & Drop** — Drag files directly from your Linux file manager (Thunar, Dolphin, PCManFM-QT, etc.) and drop them into the workspace.
- **Anti-Flashbang Theme Hook** — Integrates eight custom-tailored presets (*Nord*, *Cyberpunk*, *Sakura*, etc.) with a pre-inject loader to eliminate startup white screen flashes.
- **Native Notifications** — Sends instant system alerts via `notify-send` as soon as your files are successfully compiled and written.

---

## How It Works under the Hood

### The JS-to-C++ Bridge

When clicking **Convert**, the JavaScript frontend avoids expensive memory computations. Instead, it extracts the file's absolute path and sends a lightweight metadata payload to the C++ controller.

### Intelligent Dispatcher

The C++ backend automatically computes a non-conflicting output name (for example, appending `_converted_1.webp` if a duplicate exists) and spawns an independent child process:

```bash
magick "/path/to/input.png" "/path/to/input_converted.webp"
```

### Resource Efficiency

Background processes execute with optimal CPU multithreading usage and close to **0% idle memory footprint** when minimized.

---

## User Settings

Your local interface configuration and active theme presets are automatically persisted and stored in:

```text
~/.config/magicium/config.json
```

---

## License

Open-source and distributed under the terms of the [LICENSE](LICENSE) agreement.

Feel free to fork, hack, and expand its capabilities!