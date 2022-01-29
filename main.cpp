/**
 * @file shim.cpp
 * @author par0-git / lotuspar
 * @brief Wrapper to run instances of Minecraft with aarch64 Java from the normal launcher
 * @date 2021-2022
 */

#include <fstream>
#include <iostream>
#include <string.h>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

enum error_code {
    RES_OK = 0,
    RES_NO_MINECRAFT = 1,
    RES_TOO_MANY_ARGS = 2,
    RES_ARGUMENT_BOUNDS = 3,
    RES_NO_PATCH = 5
};

void append_argument(std::string& str, std::string argument) {
    str += " \"" + argument + "\"";
}

void append_argument(std::string& str, std::string argument_primary, std::string argument_secondary) {
    str += " \"" + argument_primary += "\" \"" + argument_secondary + "\""; 
}

/**
 * @brief Create a new Apple Silicon ready classpath from old classpath
 * 
 * @param old_classpath Old classpath
 * @param new_lwjgl_jar Path to lwjglfat.jar
 * @return std::string New classpath
 */
std::string create_classpath(std::string& old_classpath, std::filesystem::path& new_lwjgl_jar) {
    std::cout << "Creating new classpath..." << "\n";

    std::string new_classpath;

    // We need to loop over the old classpath values
    // Each part of the path is delimited by a colon (for Mac at least, Windows might use a semicolon)
    std::istringstream iss(old_classpath);
    std::string token;
    while (std::getline(iss, token, ':')) {
        if (token.find("lwjgl") != std::string::npos)
        {
            std::cout << "Skipping LWJGL JAR file " << token << "\n";
            continue;
        }

        new_classpath.append(token + ":");
    }

    // Append fat LWJGL to new classpath
    new_classpath.append(new_lwjgl_jar);

    // Return classpath
    return new_classpath;
}

int main(int argc, char **argv) {
    struct _location {
        std::filesystem::path java_home;
        std::filesystem::path minecraft;
        std::filesystem::path shim_natives, shim_fat;
    } location;

    struct _data {
        std::string classpath, new_classpath;
        std::vector<std::string> arguments;
    } data;

    // Make sure required directories exist
    location.minecraft = std::filesystem::path(getenv("HOME")) / "Library" / "Application Support" / "minecraft";
    if (location.minecraft.empty() || !std::filesystem::exists(location.minecraft)) {
        std::cout << "No Minecraft found on this device! Please make sure Minecraft has been installed and opened." << "\n";
        std::cout << "This tool is also not meant to be run standalone. Please go to a profile in the Minecraft launcher and set the Java executable to be this tool." << "\n";
        return error_code::RES_NO_MINECRAFT;
    }

    // Redirect output to a file
    std::cout << "If you're running this standalone it won't do anything!" << "\n";
    std::cout << "To use this tool you need to make a Minecraft launcher profile use this as a Java executable." << "\n";
    std::cout << "Redirecting all output to " << location.minecraft / "m1shimlog.txt" << ".\n";
    std::ofstream output(location.minecraft / "m1shimlog.txt");
    std::streambuf* initial_output = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());

    // Handle all arguments
    if (argc > 255) {
        std::cout << "Too many arguments detected! Not launching the game to be safe." << "\n";
        return error_code::RES_TOO_MANY_ARGS;
    }

    // Loop over arguments and save the ones we need to work with
    for (uint8_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-javahome") == 0) {
            if (i == 0xff || i + 1 >= argc) 
                return error_code::RES_ARGUMENT_BOUNDS;

            i++;
            location.java_home = argv[i];
            continue;
        }

        if (strcmp(argv[i], "-cp") == 0) {
            if (i == 0xff || i + 1 >= argc)
                return error_code::RES_ARGUMENT_BOUNDS;

            i++;
            data.classpath = argv[i];
            data.arguments.push_back("_CLASSPATH_POSITION_");
            continue;
        }
        
        data.arguments.push_back(argv[i]);
    }

    // Find / create required directories
    location.shim_fat = location.minecraft / "shim" / "lwjglfat.jar";
    location.shim_natives = location.minecraft / "shim" / "lwjglnatives";
    
    // Check if arm64 binaries exist
    if (!std::filesystem::exists(location.shim_fat)) {
        std::cout << "Couldn't find required binaries!" << std::endl;
        return error_code::RES_NO_PATCH;
    }

    // Print some information
    std::cout << "shim running!\n"
            << "game directory: " << location.minecraft << "\n"
            << "classpath size: " << data.classpath.size() << "\n"
            << "argument amnt: " << data.arguments.size() << "\n";

    // Make a new classpath
    data.new_classpath = create_classpath(data.classpath, location.shim_fat);

    // Prepare string to execute
    std::string java_argv = "java";

    // Use java argument if provided
    if (!location.java_home.empty()) {
        // Check if java home exists
        if (!std::filesystem::exists(location.java_home)) {
            std::cout << "JavaHome argument was provided but doesn't lead anywhere!" << "\n";
            std::cout << "provided javahome: " << location.java_home << "\n";
        } else {
            std::filesystem::path java_bin = location.java_home / "bin" / "java";
            // Check if java binary / executable exists
            if (!std::filesystem::exists(java_bin)) {
                std::cout << "JavaHome provided has no java binary!" << "\n";
            } else {
                java_argv = java_bin.string();
            }
        }
    }

    // Add LWJGL library path
    append_argument(java_argv, "-Dorg.lwjgl.librarypath=" + location.shim_natives.string());

    // Add all arguments to the string
    for (std::string& i : data.arguments) {
        if (i == "_CLASSPATH_POSITION_") {
            append_argument(java_argv, "-cp", data.new_classpath);
            continue;
        } 
        
        append_argument(java_argv, i);
    }

    // Pipe stderr to stdout
    java_argv += " 2>&1";
    
    // Put output back to normal stdout
    std::cout.rdbuf(initial_output);

    // Run Java
    FILE* pipe = popen(java_argv.c_str(), "r");
    char buffer[512];
    if (pipe) { 
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            // cout requires flush to be seen on the Minecraft Launcher log
            std::cout << buffer << std::flush;
        }
        pclose(pipe);
    } else {
        std::cout << "Failed to open pipe!" << std::endl;
    }

    return 0;
}