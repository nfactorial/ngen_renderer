# ngen_platform
The window layer: the `Window` interface a graphics backend presents to, its SDL3 implementation
`SdlWindow`, a `FakeWindow` for tests, and `ngen::Error`.

- **Spec:** [documents/specs/platform/platform.md](../documents/specs/platform/platform.md), the module's definition.
- **Target:** `ngen_platform` (alias `ngen::platform`). Public headers are in `include/ngen/platform/`
  and `include/ngen/error.h`.
- **Tests:** `ngen_platform_tests`, run with the root readme's `ctest` command.
