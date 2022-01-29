#!/bin/bash
LOCAL_CACHE=".asmacmcshim"
LOCAL_MC="$HOME/Library/Application Support/minecraft"
LOCAL_SHIM="$HOME/MinecraftShim"
REMOTE_SHIM="https://github.com/lotuspar/MinecraftAppleSiliconShim/releases/download/v1.0.0/shim"
REMOTE_REQUIREMENTS="https://raw.githubusercontent.com/lotuspar/MinecraftAppleSiliconShim/main/requirements/requirements.tgz"

# Make sure Minecraft is installed
if [ ! -d "$LOCAL_MC" ]
then
    echo "Minecraft wasn't found! Please make sure you've installed and ran Minecraft at least once."
    exit 1
fi

# Create folders
echo "Preparing to install shim..."
mkdir "$LOCAL_CACHE"
mkdir "$LOCAL_MC/shim"

# Download requirements
echo "Downloading requirements..."
curl $REMOTE_REQUIREMENTS -o "$LOCAL_CACHE/requirements.tgz"

if [ ! -d $LOCAL_CACHE/requirements.tgz]
then
    echo "Failed to download requirements! Please check your internet connection."
    exit 2
fi

# Extract requirements 
echo "Extracting requirements..."
tar -xvzf "$LOCAL_CACHE/requirements.tgz" -C "$LOCAL_MC/shim"

# Remove quarantine attribute
xattr -r -d com.apple.quarantine "$LOCAL_MC/shim/lwjglnatives/"*.dylib

# Download shim
echo "Downloading shim..."
curl -L "$REMOTE_SHIM" -o $LOCAL_SHIM
chmod +x "$LOCAL_SHIM"
xattr -r -d com.apple.quarantine "$LOCAL_SHIM"

# Clean up
echo "Cleaning installer files..."
rm -rf "$LOCAL_CACHE"

echo "Shim should be ready for use at $LOCAL_SHIM!"
echo "You now need to go to the Minecraft launcher and set the Java executable to location of the shim."
echo "Move it anywhere you want if you need it somewhere else. (make sure to update the profile first though!)"