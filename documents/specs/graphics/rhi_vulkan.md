# RHI Vulkan backend — Spec

> - **Template:** v1 · **ID:** rhi-vulkan
> - **Status:** built
> - **Blocked by:** [RHI](rhi.md), [Platform](../platform/platform.md)
> - **Background (not needed to build):** Feature: [ngen](../README.md) · Design: [renderer](../../design/renderer.md) · Technical: [renderer](../../technical/renderer.md) · Inputs: none

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
- Primitive restart is enabled for `TriangleStrip` pipelines and disabled for list topologies:
  Metal cannot disable it (MoltenVK warns when asked to), and core Vulkan forbids it on lists. It
  has no effect until indexed draws exist.
- Viewport and scissor are dynamic pipeline state. Every `beginRendering` sets both to the full
  target image, so a `setViewport` or `setScissor` lasts until the end of its pass; one made
  outside a pass is recorded and overridden by the next `beginRendering`.
- A `Load` pass while the swap chain image's tracked state is `Undefined` uses the `DontCare`
  pass, because its contents are undefined; the image keeps no record of being rendered across
  frames.
- Every render pass starts and ends with its image in `COLOR_ATTACHMENT_OPTIMAL`. A pass whose
  load is `Load` keeps the image's contents, and any other load discards them. The image's
  layout changes only through barriers: those `barrier` records, the one `beginRendering` makes
  for itself, and `endFrame`'s transition. Each render pass keeps a subpass dependency from
  `VK_SUBPASS_EXTERNAL`, so a pass that follows another with no barrier between them is still
  ordered after it.
- Each barrier is one `vkCmdPipelineBarrier` with one image barrier, mapping each state to a
  layout, a stage and an access mask: `Undefined` → `UNDEFINED`, `COLOR_ATTACHMENT_OUTPUT`,
  none; `ColorAttachment` → `COLOR_ATTACHMENT_OPTIMAL`, `COLOR_ATTACHMENT_OUTPUT`, colour
  attachment read and write; `ShaderRead` → `SHADER_READ_ONLY_OPTIMAL`, `FRAGMENT_SHADER`,
  shader read; `Present` → `PRESENT_SRC_KHR` (`COLOR_ATTACHMENT_OPTIMAL` on a
  `WindowSupport::None` device), `BOTTOM_OF_PIPE`, none. A barrier from `Undefined` waits on
  `COLOR_ATTACHMENT_OUTPUT`, because the acquire semaphore's wait is at that stage.
- `VulkanDevice` and its command list are `final` and derive directly from `ngen::rhi::Device`
  and `ngen::rhi::CommandList`.
- The swap chain prefers `B8G8R8A8_UNORM` with sRGB non-linear colour space, else the first
  offered format; it prefers `FIFO` presentation (always available, so frame pacing follows the
  display). It applies the surface's current transform. Composite alpha is opaque when the
  surface supports it, else the first mode it offers.
- Vulkan results map to `ngen::ErrorCode`: incompatible driver, a missing layer, extension or
  feature, and an unsupported format are `Unsupported`; out-of-memory results are
  `OutOfMemory`; device and surface loss are `DeviceLost` and `SurfaceLost`; any other failure is
  `Internal`.
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
  rejected. `create` with `WindowSupport::Expected` on a platform with no surface extension
  returns `Unsupported`.
- `createShader` with bytecode that is not SPIR-V (a size not a multiple of 4, or no SPIR-V
  magic number) returns `InvalidArgument`. Destroying a shader frees it at once.
- A pipeline with `PolygonMode::Line` enables `fillModeNonSolid` where the device has it, and
  returns `Unsupported` where it does not.
- Beyond the RHI's misuse rules, the command list skips with an `Error` message: `setScissor`
  with a negative x or y, `bindVertexBuffer` at or past `maxVertexInputBindings`, `draw` with
  no pipeline bound, and `draw` while a binding the pipeline reads is unbound.
- `attachWindow` with a `NativeWindow` of a kind the build's platform does not use, or whose
  surface the device's queue family cannot present to, returns `Unsupported`.
- **Swap chain recreation:** the swap chain is recreated, after the GPU finishes its frames in
  flight, when `onResized` reports a size different from the swap chain's, when acquiring or
  presenting returns `VK_ERROR_OUT_OF_DATE_KHR`, and when presenting returns `VK_SUBOPTIMAL_KHR`.
  An acquire returning `VK_SUBOPTIMAL_KHR` still renders that frame. Recreation passes the old
  swap chain as `oldSwapchain` and rebuilds the framebuffers. Recreation for a new size happens
  at the next `beginFrame`. An out-of-date acquire recreates and acquires once more; if that
  fails too, `beginFrame` returns `nullptr` and recreation stays pending. While the window size
  is non-zero but no swap chain exists, each `beginFrame` tries to create one.
- **Abandoned frames:** when a call fails after an image is acquired (the wait on the image's
  earlier fence, command pool reset, command buffer begin or end, fence reset, or submit), and
  the failure is not device loss, the swap chain is marked for recreation and an empty submit
  waits on the acquire's semaphore and signals the slot's fence. If that recovery fails, the
  device is treated as lost.
- While the window size is 0 in either dimension, no swap chain exists and `beginFrame` returns
  `nullptr`; the first non-zero size recreates it.
- `onSuspended` waits for the GPU, then destroys the swap chain and the surface; `onResumed`
  creates a new surface from the handles given (replacing any that remains) and a new swap
  chain. `onSuspended` while a frame is being recorded takes effect once `endFrame` has
  submitted; `onResumed` while recording sends an `Error` message and returns
  `InvalidArgument`.
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
- A frame that records `barrier({SwapChainImage, Undefined, ColorAttachment})`, a `Clear` pass,
  `barrier({SwapChainImage, ColorAttachment, ColorAttachment})` and a `Load` pass that draws the
  test pipeline presents with no `Warning` or `Error` message; so does a frame that records
  `beginRendering` with no barrier first, one that records no pass, and, in a later frame than
  the first, a `Load` pass recorded while the image is `Undefined`.

## Pending changes

### Test image barrier target  (in [RHI](rhi.md))
