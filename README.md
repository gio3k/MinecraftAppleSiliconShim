# MinecraftAppleSiliconShim
Run the ARM64 version of Minecraft through the stock launcher

## Requirements
To use this you need the ARM64 LWJGL natives & fat binary. You can get this from [m1-multimc-hack](https://github.com/yusefnapora/m1-multimc-hack).\
These need to be in a folder called "shim" in the Minecraft data directory (~/Library/Application Support/minecraft)\
\
The end result should be 2 paths existing like this:\
`~/Library/Application Support/minecraft/shim/lwjglnatives/` (filled with .dylib files)\
`~/Library/Application Support/minecraft/shim/lwjglfat.jar` (one file)\
\
After the files are there, remove the quarantine flag from the dylibs by executing `xattr -r -d com.apple.quarantine ~/Library/Application Support/minecraft/shim/lwjglnatives/*.dylib` in the Terminal.

## Usage
After getting the requirements just set the Java executable in your Minecraft profile to the shim!

## Quirks and fixes
### Troubleshooting
Bugs in the shim won't be visible in the Minecraft launcher. To see them look at "m1shimlog.txt" in the Minecraft data directory.

### Java version changing
By default the shim will use the first Java found in the path (the one used when running `java` in the Terminal)\
To change the Java version running the game, pass `-javahome (path to java home directory)` to the game arguments. You can do this right below where you set the profile Java executable.\
To find Java home directories on your system, run `/usr/libexec/java_home -V` in the Terminal.
