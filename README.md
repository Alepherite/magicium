# **🖼️ Magicium Converter**

## **✨ Features**

* **Hybrid Architecture:** The best of both worlds. A modern, fluid HTML5/JS/CSS user interface powered by WebKitGTK, sitting on top of a lightning-fast, compiled C++ backend.  
* **Zero Base64 Overhead:** Unlike typical web-based converters that choke on large files due to browser-side canvas encoding, Magicium delegates all processing directly to the system's native magick binary via physical file paths.  
* **Asynchronous Execution:** Converts images in an isolated, non-blocking background thread (g\_spawn\_command\_line\_async), ensuring zero UI freezing even when processing massive resolutions.  
* **Full Drag & Drop Support:** Drop any image directly from your native Linux File Manager (such as Thunar, Dolphin, or PCManFM-QT) straight into the app window.  
* **Flawless Theme Integration:** Ships with 8 beautiful, custom-tailored presets (*Light, Dark, Sunset, Nord, Cyberpunk, Forest, Sakura, and Matrix*). Features a specialized pre-inject theme hook to **completely eliminate "flashbangs"** (white screen flashes) during cold boot.  
* **Desktop Notifications:** Instantly alerts you via native system notifications (notify-send) once your converted file is safely written to disk.

## **🛠️ Prerequisites & Dependencies**

Magicium is tailor-made for Linux. Before building, install the required development libraries for your distribution:

### **1\. Debian / Ubuntu / Linux Mint:**

sudo apt install build-essential libgtk-3-dev libwebkit2gtk-4.1-dev imagemagick libnotify-bin

### **2\. Fedora / RHEL:**

sudo dnf install gcc-c++ make gtk3-devel webkit2gtk4.1-devel ImageMagick libnotify

### **3\. Arch Linux / CachyOS / Manjaro:**

sudo pacman \-S base-devel gtk3 webkit2gtk-4.1 imagemagick libnotify

## **🚀 Installation & Running**

### **Option A: One-Click Automated Setup (Recommended)**

If you want a hassle-free setup without dealing with compiler commands, run our setup assistant. This script will automatically compile the app and add a handy shortcut to your application menu:

git clone \[https://github.com/Alepherite/magicium.git\](https://github.com/Alepherite/magicium.git)    
cd magicium/    
chmod \+x install.sh    
./install.sh

*Once finished, you can find and launch **Magicium Converter** straight from your system's application launcher (like Rofi, Bemenu, or your desktop menu).*

### **Option B: Manual Compilation**

Alternatively, you can compile, build, and run the binary manually using this single command block:

git clone \[https://github.com/Alepherite/magicium.git\](https://github.com/Alepherite/magicium.git)    
cd magicium/    
make    
build/magicium

## **⚙️ How It Works under the Hood**

* **UI to Backend Bridge:** When clicking Convert & Save, the JavaScript UI avoids expensive data-URI calculations. Instead, it extracts the target format and the original physical absolute path of the image, then fires a lightweight payload to the C++ controller.  
* **Command Dispatcher:** The C++ backend decodes the file path, computes a safe, non-conflicting output filename (e.g., appending suffixes like \_converted\_1.webp if a file with the same name already exists), and spawns an independent child process running:  
  magick "/path/to/input.png" "/path/to/input\_converted.webp"

* This ensures maximum CPU multithreading utilization with close to 0% idle RAM usage when the app runs in the background.

## **📝 User Settings & Persistence**

Your custom layout selections (such as selected theme and preferences) are automatically saved to your system's configuration standard directory:

\~/.config/magicium/config.json

## **📄 License**

This project is open-source and available under the terms of the [LICENSE](http://docs.google.com/LICENSE) agreement. Feel free to fork, hack, and expand its capabilities\!