# Example application — Spec

> - **Template:** v1 · **ID:** example
> - **Status:** ready
> - **Blocked by:** [RHI Vulkan backend](../graphics/rhi_vulkan.md), [RHI](../graphics/rhi.md), [Platform](../platform/platform.md)
> - **Background (not needed to build):** Feature: [ngen](../README.md) · Design: [renderer](../../design/renderer.md) · Technical: none · Inputs: none

## Goal

The example application is ngen's visual check: a window that draws a vertex-coloured
triangle through ngen, and keeps drawing it as the window is resized, minimised and restored.
It is a composition root: it creates the SDL window and the Vulkan device and ties them
together.

## Scope

**In:**
- The window, device, triangle resources and frame loop of the example.
- Forwarding the window's events to the device.
- Removing `ngen_vulkan`, which nothing uses once the example is ported.

**Out (non-goals):**
- Input beyond closing the window.
- Any rendering feature beyond the triangle.

## Constraints and key decisions

- The example uses `ngen::platform::SdlWindow` and `ngen::rhi::vulkan::VulkanDevice`, with
  validation on, and reaches the GPU only through `ngen::rhi::Device` and `CommandList`.
- The window is 640 × 480, resizable, titled "nGen - Vulkan Test".
- The triangle's vertices are positions (`R32G32Float`, location 0) and colours
  (`R32G32B32Float`, location 1), interleaved and packed in binding 0 (position at offset 0,
  colour at offset 8, stride 20): (0, −0.5) red, (0.5, 0.5) green,
  (−0.5, 0.5) blue. Its shader is `example/shaders/triangle.slang`, with entry points
  `vertexMain` and `fragmentMain`, compiled to `shaders/triangle.spv` beside the executable as
  the build does today. The pipeline culls nothing, so the winding has no effect.
- Each frame clears to black and draws the triangle over the whole window.
- The example prints nothing when all is well, and exits with status 0 when the window is
  closed. Any failure, at start-up or after it (fetching the window's handles, `onResumed`,
  `beginFrame` or `endFrame` returning an error), is printed once to `stderr`; the example then
  shuts down as the guardrails order and exits with status 1. `beginFrame` returning no command
  list is not a failure.
- The frame loop passes every SDL event, polled or returned by `SDL_WaitEventTimeout`, to
  `SdlWindow::handleEvent`, then drains the window's `pollEvents` and forwards its `Resized`,
  `Suspended` and `Resumed` events to the device, and stops on `CloseRequested`. On `Resumed` it
  fetches fresh handles with `nativeWindow()` before calling `onResumed`, because handles from
  before a suspension are invalid.
  While `beginFrame` returns no command list, it waits for events (`SDL_WaitEventTimeout`, 100 ms)
  instead of spinning.
- The example links SDL3 only.

## Modules and interfaces

### Example  (example/main.cpp, example/render.cpp)

Starts SDL, creates the window and device, loads the triangle, runs the frame loop, and shuts
down in reverse order.

**Acceptance:**
- Started with the compiled shader missing, the example prints an error naming
  `shaders/triangle.spv` and exits with status 1.
- Run normally and closed, it exits with status 0 and prints nothing to `stdout` or `stderr`.

## Build

**Milestone:** [Triangle on the RHI](../README.md)

**Read first:** `ngen_platform/include/ngen/platform/`, `ngen_rhi/include/ngen/rhi/`,
`ngen_rhi_vulkan/include/ngen/rhi/vulkan/vulkan_device.h`; `example/` as it is.

**Touchpoints:**

| Path / symbol | Create / modify | Kind |
|---|---|---|
| `example/main.cpp`, `example/render.cpp`, `example/render.h`, `example/CMakeLists.txt` | modify | code |
| `ngen_vulkan/` | delete | code |
| `CMakeLists.txt` | modify (remove `ngen_vulkan`) | config |
| `readme.md` | modify (modules, dependencies, running the tests) | config |

**Fixed:**
- The window, triangle and behaviour in *Constraints*.
- `example/main.cpp` opens with `// Spec: example. Behaviour changes go through it.`

**Free:** how the code is split between `main.cpp` and `render.cpp`.

**Guardrails:**
- The device is destroyed before the window, and the window before `SDL_Quit`.
- Every handle the example creates is destroyed before the device.

**Done when:**
- *Agent-checkable:* the missing-shader criterion holds when run · a grep of the source tree and
  CMake files finds no `ngen_vulkan` · the project configures and builds from a
  clean build folder · `ctest` passes.
- *Human check:* the milestone check: run `build/windows/example/Debug/example.exe` from its
  folder; the triangle draws; drag the window larger and smaller, minimise and restore it, and the
  triangle keeps drawing, centred and filling the same share of the window at every size, with
  nothing printed; closing the window exits with status 0 and nothing printed.

**Delegation:** Sonnet implementer, light review. One application, nothing else relies on it.
