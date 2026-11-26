# ngen_rhi
The rendering hardware interface: a backend-neutral API for creating GPU resources, recording
draw commands and presenting frames, with a `FakeDevice` for tests.

- **Spec:** [documents/specs/graphics/rhi.md](../documents/specs/graphics/rhi.md), the module's definition.
- **Target:** `ngen_rhi` (alias `ngen::rhi`). Public headers are in `include/ngen/rhi/`.
- **Tests:** `ngen_rhi_tests`, run with the root readme's `ctest` command.
