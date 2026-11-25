NGEN Rendering Example
This application demonstrates how to use the rendering engine to present a scene to the user.

Shaders are written in [Slang](https://shader-slang.org/) and live in the `shaders` folder. The build
compiles them to SPIR-V with `slangc` (shipped with the Vulkan SDK) and copies them beside the executable.
