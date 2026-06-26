#!/bin/bash

# Run uninstall.sh
if [ -f "uninstall.sh" ]; then
    chmod +x uninstall.sh
    ./uninstall.sh
else
    echo "uninstall.sh not found. Skipping uninstallation."
fi

# Clone the repository
git clone https://github.com/Alepherite/magicium.git 
if [ $? -ne 0 ]; then
    echo "Failed to clone repository. Aborting update."
    exit 1
fi

# Move into the cloned repository and install
cd magicium
if [ $? -ne 0 ]; then
    echo "Failed to change directory to magicium. Aborting update."
    exit 1
fi

chmod +x install.sh
if [ $? -ne 0 ]; then
    echo "Failed to make install.sh executable. Aborting update."
    exit 1
fi

./install.sh
if [ $? -ne 0 ]; then
    echo "Failed to run install.sh. Aborting update."
    exit 1
fi

# Clean up temporary directory
cd ..
rm -rf magicium_temp

echo "Update complete."
