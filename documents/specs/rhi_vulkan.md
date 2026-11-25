# RHI Vulkan backend — Spec

> - **Template:** v1 · **ID:** rhi-vulkan
> - **Status:** ready
> - **Blocked by:** [RHI](rhi.md), [Platform](platform.md)
> - **Background (not needed to build):** Feature: [ngen](README.md) · Design: [renderer](../design/renderer.md) · Technical: [renderer](../technical/renderer.md) · Inputs: none

## Goal

`ngen_rhi_vulkan` implements the RHI's `Device` and `CommandList` on Vulkan. It creates the
instance, device and swap chain from the platform's native window handles, allocates memory
through VMA, keeps the swap chain matching the window across resizes, minimising and
suspension, and reports validation messages through the device's message sink. A device can
also be created with no window, for tests.

## Scope

**In:**
- `VulkanDevice` and its command list, implementing every `Device` and `CommandList` contract.
- Instance, physical device and logical device setup on Vulkan 1.1, with portability (MoltenVK)
  and validation.
- Surface creation from `Win32Handles`, `AndroidHandles` and `AppleHandles`.
- Swap chain creation and recreation.
- VMA for buffer memory.
- A smoke test on a real device with validation on.

**Out (non-goals):**
- Dynamic rendering and synchronization2: this milestone uses the Vulkan 1.1 path only.
- Textures, descriptor sets, index buffers, compute.
- Choosing among several GPUs by any rule beyond the one in *Constraints*.

## Constraints and key decisions

- Every public type is in namespace `ngen::rhi::vulkan`. The one public header is
  `ngen_rhi_vulkan/include/ngen/rhi/vulkan/vulkan_device.h`; Vulkan types appear in no public
  header.
- The CMake target is `ngen_rhi_vulkan`, alias `ngen::rhi_vulkan`, depending on `ngen::rhi`,
  `ngen::platform` and Vulkan. It does not depend on SDL.
- The instance requests API version 1.1. Shaders are SPIR-V 1.3.
- Instance extensions: `VK_KHR_surface` and the one surface extension for the window kind the
  build's platform uses (`VK_KHR_win32_surface`, `VK_KHR_android_surface` or
  `VK_EXT_metal_surface`), only when a window may be attached (see `create`);
  `VK_KHR_portability_enumeration`, with the enumerate-portability flag, whenever the loader
  offers it; `VK_EXT_debug_utils` when validation is on.
- Device extensions: `VK_KHR_swapchain` when a window may be attached, and
  `VK_KHR_portability_subset` whenever the device offers it.
- With validation on, `VK_LAYER_KHRONOS_validation` is enabled and a debug messenger for
  warnings and errors (general, validation and performance types) is created, and also chained
  into instance creation; each message goes to the sink as `Warning` or `Error`.
- A physical device is suitable when it has a graphics queue family, supports Vulkan 1.1, and,
  when a window may be attached, has the swap chain extension and a graphics family that can
  present. A discrete GPU is preferred over
  any other suitable device; otherwise the first suitable device is used.
- The device has one queue, from one family used for both graphics and presentation, chosen at
  `create` because the surface arrives later. With `WindowSupport::Expected` it is the first
  graphics family the platform reports can present: on Windows,
  `vkGetPhysicalDeviceWin32PresentationSupportKHR`; on Android and Apple, any graphics family.
  `attachWindow` and `onResumed` confirm with `vkGetPhysicalDeviceSurfaceSupportKHR`.
- Memory for buffers comes from VMA (VulkanMemoryAllocator, fetched with `FetchContent`, pinned
  to tag `v3.3.0`), host-visible and coherent.
- Each frame slot has a command pool and buffer, an image-available semaphore and an in-flight
  fence; each swap chain image has its own render-finished semaphore.
- Rendering uses a `VkRenderPass` per distinct (colour format, `LoadOp`, `StoreOp`), created on
  first use and cached, with one framebuffer per swap chain image.
- **Pipeline variants:** a pipeline handle holds one `VkPipeline` per colour format it has been
  used with, each created against a render pass of that format with load `Clear` and store
  `Store` (compatible with every cached pass of that format). The pipeline keeps its own copy of
  its shaders' bytecode and entry points, so destroying a shader handle never affects a pipeline
  made from it. `createPipeline` creates the variant for the swap chain's current format, or for
  `B8G8R8A8_UNORM` when there is no swap chain; after a recreation changes the format, binding a
  pipeline creates the new format's variant if it has none, and its handle stays valid.
- Viewport and scissor are dynamic pipeline state.
- The swap chain prefers `B8G8R8A8_UNORM` with sRGB non-linear colour space, else the first
  offered format; it prefers `FIFO` presentation (always available, so frame pacing follows the
  display). It applies the surface's current transform.
- ngen prints nothing itself; everything is reported through the message sink or returned.

## Modules and interfaces

### VulkanDevice  (ngen_rhi_vulkan/include/ngen/rhi/vulkan/vulkan_device.h)

The Vulkan implementation of `ngen::rhi::Device`, owning every Vulkan object it creates.

**Interface:**
- `enum class WindowSupport { None, Expected }`
- `static std::expected<std::unique_ptr<ngen::rhi::Device>, ngen::Error> VulkanDevice::create(const ngen::rhi::DeviceDescription&, WindowSupport)`:
  `Expected` enables the surface and swap chain extensions so `attachWindow` may be called;
  `None` makes a device that never presents (for tests and offscreen use), whose
  `attachWindow` returns `Unsupported`.
- Every `Device` and `CommandList` member, as the RHI specifies.

**Requirements:**
- `create` with validation requested when `VK_LAYER_KHRONOS_validation` is not installed returns
  `Unsupported`, saying the layer is missing.
- `create` with no suitable physical device returns `Unsupported`, saying why devices were
  rejected.
- `attachWindow` with a `NativeWindow` of a kind the build's platform does not use, or whose
  surface the device's queue family cannot present to, returns `Unsupported`.
- **Swap chain recreation:** the swap chain is recreated, after the GPU finishes its frames in
  flight, when `onResized` reports a size different from the swap chain's, when acquiring or
  presenting returns `VK_ERROR_OUT_OF_DATE_KHR`, and when presenting returns `VK_SUBOPTIMAL_KHR`.
  An acquire returning `VK_SUBOPTIMAL_KHR` still renders that frame. Recreation passes the old
  swap chain as `oldSwapchain` and rebuilds the framebuffers.
- While the window size is 0 in either dimension, no swap chain exists and `beginFrame` returns
  `nullptr`; the first non-zero size recreates it.
- `onSuspended` waits for the GPU, then destroys the swap chain and the surface; `onResumed`
  creates a new surface from the handles given and a new swap chain.
- **Device loss:** `VK_ERROR_DEVICE_LOST` from any Vulkan call on the frame path (fence wait,
  acquire, submit, present) is returned as `DeviceLost`, and every later `beginFrame` returns
  `DeviceLost`.
- **Surface loss:** `VK_ERROR_SURFACE_LOST_KHR` from acquire or present destroys the swap chain
  and surface (after the GPU finishes its frames in flight) and is returned as `SurfaceLost`;
  `beginFrame` then returns `nullptr` until `onResumed` succeeds.
- The destructor waits for the GPU and destroys everything it created, including retired
  resources, in reverse order of creation, with no validation messages.

**Acceptance (smoke test, real Vulkan, validation on, `WindowSupport::None`):**
- Creating the device, a 1 KB vertex buffer, writing 36 bytes into it, a shader from the
  test's own Slang module (a vertex and a fragment entry point, `vertexMain` and
  `fragmentMain`, compiled by the build to SPIR-V 1.3), and a pipeline using both entries, then destroying them and the device, sends no `Warning` or `Error` message to the sink.
- `writeBuffer` of 2 KB into the 1 KB buffer returns `InvalidArgument`.
- `attachWindow` returns `Unsupported`.

**Acceptance (window test, real Vulkan, validation on, `WindowSupport::Expected`):**
- With a real `ngen::platform::SdlWindow` of 640 × 480 attached, the device presents 3 frames
  (each clearing the image and drawing the test pipeline); after `SDL_SetWindowSize` to
  800 × 600 and the resulting `onResized`, it presents 3 frames at 800 × 600
  (`swapChainSize()`); after `onSuspended`, `beginFrame` returns `nullptr`; after `onResumed`
  with the window's handles, it presents 3 more frames. No `Warning` or `Error` message reaches
  the sink.

## Build

**Milestone:** [Triangle on the RHI](README.md)

**Read first:** `ngen_rhi/include/ngen/rhi/`; `ngen_platform/include/ngen/platform/window.h`;
`ngen_vulkan/source/` (the Vulkan code being
ported: instance and device setup, the debug messenger, swap chain, frame synchronisation in
`renderer.cpp`, pipeline description, render pass).

**Touchpoints:**

| Path / symbol | Create / modify | Kind |
|---|---|---|
| `CMakeLists.txt` | modify (add `ngen_rhi_vulkan`) | config |
| `cmake/dependencies.cmake` | modify (VMA) | config |
| `ngen_rhi_vulkan/` (CMakeLists, header, sources, `tests/` with its Slang test shader) | create | code |

**Fixed:**
- `VulkanDevice::create`, `WindowSupport` and the header path.
- `VulkanDevice` and its command list derive directly from `ngen::rhi::Device` and
  `ngen::rhi::CommandList`, and nothing derives from them.
- The main file opens with `// Spec: rhi-vulkan. Behaviour changes go through it.`
- The test target is `ngen_rhi_vulkan_tests`. The smoke test and the window test are registered
  with the CTest label `gpu`, and find their compiled shader through a path the build passes in.
  Only the test target links `ngen_platform`'s SDL implementation; the library does not.

**Free:** the internal class and file split, how resources are stored (it may reuse
`HandlePool`), and further tests.

**Guardrails:**
- The swap chain's format, extent, image count and present mode are read from the surface at
  each (re)creation, never kept from the first one.
- Each frame waits on its own slot's fence before reusing that slot's command buffer, and on the
  fence of any earlier frame still using the acquired image.
- A fence is reset only immediately before the submit that will signal it.
- `ngen_vulkan` is left unchanged.

**Done when:**
- *Agent-checkable:* every requirement and acceptance criterion above is covered by a test
  where one can run on this machine (device and surface loss, and out-of-date results, cannot be
  forced; their handling is checked in review) · `ctest` passes, including both `gpu` tests on
  this machine · `ngen_rhi_vulkan`'s
  public header includes no Vulkan header (grep).
- *Human check:* none of its own; presenting, resizing and minimising are the milestone check.

**Delegation:** deep implementer, full review. Vulkan synchronisation and swap chain lifetime, where subtle mistakes pass the build.
