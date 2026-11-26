# example
An application that draws a vertex-coloured triangle through ngen: ngen's visual check.

- **Spec:** [documents/specs/examples/example.md](../documents/specs/examples/example.md), the application's definition.
- **Target:** `example`, built when `NGEN_BUILD_EXAMPLE` is on. Run it from its build folder, as the root readme says.
- **Shaders** are written in [Slang](https://shader-slang.org/) and live in `shaders/`. The build compiles them to
  SPIR-V with `slangc` (shipped with the Vulkan SDK) and copies them beside the executable.
- **Tests:** `example_missing_shader` (`tests/missing_shader.cmake`).
