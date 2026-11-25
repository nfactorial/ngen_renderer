# nGen Renderer
Rendering library for nGen game engine.

nGen uses the Vulkan graphics API for cross-platform rendering. The ngen_vulkan library is intended
to be a low-level wrapper around the Vulkan API to allow for re-use and refactoring of the higher
level libraries.

See [documents/design/renderer.md](documents/design/renderer.md) for the planned structure.

# Dependencies
- **CMake 3.24 or later.**
- **Vulkan SDK** from [LunarG](https://vulkan.lunarg.com/sdk/home). The installer sets the `VULKAN_SDK`
  environment variable, which CMake uses to find Vulkan and the `slangc` shader compiler.
- **SDL2.** If CMake can find an installed SDL2 (through its CMake package, e.g. by adding the install
  folder to `CMAKE_PREFIX_PATH`), that is used. Otherwise SDL2 is downloaded and built automatically.

The Vulkan library currently depends on SDL for creating the display surface. A goal is to remove this
dependency so the Vulkan renderer can be used with or without SDL in the future.

# Building
```
cmake --preset windows        # or: cmake --preset macos
cmake --build --preset windows
```
The example is written to `build/windows/example/Debug/`, with its compiled shaders in a `shaders`
folder beside it. Run it from that folder, as it loads the shaders by relative path.

Options (pass with `-D<option>=ON|OFF`):
- `NGEN_BUILD_EXAMPLE` (default ON): the example application.

macOS support through MoltenVK has not been tested yet. The instance and device enable the Vulkan
portability extensions MoltenVK requires, but the build has only been run on Windows.

# Development
This project is planned and built with [Claude Code](https://claude.com/claude-code) and the
[nfx-workflow](https://github.com/nfactorial/nfx-workflow) plugin. The design, technical
documents and specs live under [documents/](documents/), and each module's code names the spec it
was built from.
