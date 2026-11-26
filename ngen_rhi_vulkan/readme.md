# ngen_rhi_vulkan
The Vulkan implementation of the RHI, `VulkanDevice`.

- **Spec:** [documents/specs/graphics/rhi_vulkan.md](../documents/specs/graphics/rhi_vulkan.md), the module's definition.
- **Target:** `ngen_rhi_vulkan` (alias `ngen::rhi_vulkan`). Public headers are in `include/ngen/rhi/vulkan/`.
- **Tests:** `ngen_rhi_vulkan_tests`. The tests on a real Vulkan device carry the `gpu` label:
  `ctest --test-dir build/windows -C Debug -LE gpu` skips them on a machine with no GPU, and `-L gpu` runs only them.
