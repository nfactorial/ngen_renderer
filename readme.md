# nGen Renderer
Rendering library for nGen game engine.

nGen uses the Vulkan graphics API for cross-platform rendering, behind a small graphics-API-neutral
interface (the RHI).

# Modules
- [`ngen_platform`](ngen_platform/readme.md): the `Window` interface, its SDL3 implementation `SdlWindow` and a `FakeWindow` for tests.
- [`ngen_rhi`](ngen_rhi/readme.md): the RHI (`Device`, `CommandList`, handles and descriptions) and a `FakeDevice` for tests.
- [`ngen_rhi_vulkan`](ngen_rhi_vulkan/readme.md): the Vulkan implementation of the RHI, `VulkanDevice`.
- [`example`](example/readme.md): an application that draws a vertex-coloured triangle through the above.

See [documents/design/renderer.md](documents/design/renderer.md) for the planned structure.

# Dependencies
- **CMake 3.24 or later.**
- **Vulkan SDK** from [LunarG](https://vulkan.lunarg.com/sdk/home). The installer sets the `VULKAN_SDK`
  environment variable, which CMake uses to find Vulkan and the `slangc` shader compiler.
- **SDL3.** If CMake can find an installed SDL3 (through its CMake package, e.g. by adding the install
  folder to `CMAKE_PREFIX_PATH`), that is used. Otherwise SDL3 is downloaded and built automatically.

# Building
```
cmake --preset windows        # or: cmake --preset macos
cmake --build --preset windows
```
The example is written to `build/windows/example/Debug/`, with its compiled shaders in a `shaders`
folder beside it. Run it from that folder, as it loads the shaders by relative path.

# Running the tests
```
ctest --test-dir build/windows -C Debug --output-on-failure     # macOS: ctest --test-dir build/macos
```
This includes `example_missing_shader`, which runs the example without its compiled shader and checks it
reports the missing file and exits with status 1.

Options (pass with `-D<option>=ON|OFF`):
- `NGEN_BUILD_EXAMPLE` (default ON): the example application.

macOS support through MoltenVK has not been tested yet. The instance and device enable the Vulkan
portability extensions MoltenVK requires, but the build has only been run on Windows.

# Development
This project is planned and built with [Claude Code](https://claude.com/claude-code) and the
[nfx-workflow](https://github.com/nfactorial/nfx-workflow) plugin. The design, technical
documents and specs live under [documents/](documents/), and each module's code names the spec it
was built from.
