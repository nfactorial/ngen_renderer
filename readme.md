# nGen Renderer
Rendering library for nGen game engine.

nGen uses the Vulkan graphics API for cross-platform rendering. The ngen_vulkan library is intended
to be a low-level wrapper around the Vulkan API to allow for re-use and refactoring of the higher
level libraries.

The display library sits on top of the rendering library, the display library is aimed at simplifying
the rendering interface into higher order objects for use by the game itself.

# DEPENDECIES
nGen uses SDL to manage system initialization, you must have SDL available on your system in-order for
compilation to succeed. Please see https://www.libsdl.org/

# SDL2
The renderer currently makes use of SDL2, please ensure the SDL2 SDK has been installed and ensure
the environment variable SDL2DIR points to the install location of the SDL2 SDK.

# Vulkan
The Vulkan graphics library is used to provide rendering functionality for the framework.

Please ensure the environment variable VULKAN_SDK is defined and points to the install location of your
Vulkan SDK.

The build configuration is still in the process of being established along with the initial
library implementation. The build configuration will be stabalised once the library is in a usable state.

## Windows
To build this library on Windows, download the latest [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
and install it onto your machine. Ensure the VULKAN_SDK environment variable has been configured to
point at the SDK location.

## OSX
To build this library on OSX, download the latest [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) and
install it onto your machine. You will need to configure an environment variable VULKAN_SDK to contain
the path to the installed folder.
```
VULKAN_SDK="${HOME}/Library/Frameworks/vulkan-sdk/macOS"
```
### JetBrains C-Lion
When developing with C-Lion, if you do not wish to have a system-wide environment variable. You may
configure the environment root by navigating to Preferences/Build,Execution,Deployment and choosing
the CMake sub-option. Here you may specify environment variables that are exclusive to your project.
 
If you change the system environment variables, ensure you restart the JetBrains toolbox application.
Otherwise the environment variable changes will not propagate to the application.
