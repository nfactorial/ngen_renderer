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

@ Vulkan
The Vulkan graphics library is used to provide rendering functionality for the framework.

## Windows
To build this library on Windows, download the latest [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
and install it onto your machine. Ensure the VULKAN_SDK environment variable has been configured to
point at the SDK location.

## OSX
To build this library on OSX, download the latest [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) and
install it onto your machine. You will need to configure an environment variable VULKAN_SDK to contain
the path to the installed folder.
