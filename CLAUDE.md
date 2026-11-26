# ngen_renderer

Supports both Windows and macOS: every change must build and pass its tests on both. macOS runs
Vulkan through MoltenVK; the Apple platform code (the `CAMetalLayer` from SDL's Metal view) is
written but not yet verified on a Mac. Run the checks for whichever machine you are on; changes
made on one machine are checked on the other when the work moves there.

Work follows the nfx workflow (https://github.com/nfactorial/nfx-workflow): specs live in
`documents/specs/`, and code changes keep them true.

## Checks before a commit

Run the commands for the machine you are on.

**Windows** (PowerShell):

```
cmake --preset windows
cmake --build --preset windows
ctest --test-dir build/windows -C Debug --output-on-failure
```

**macOS:**

```
cmake --preset macos
cmake --build --preset macos
ctest --test-dir build/macos --output-on-failure
```

## C++ guidelines

Style is free: readable code is fine. These two affect code quality, so follow them:

- **`const` by-value parameters in definitions.** When a definition doesn't modify a by-value
  parameter, mark it `const` there (`Error makeError(const ErrorCode code, std::string message)`).
  Leave the header declaration without it, because it doesn't affect callers.
- **Pass large objects by `const&` when only read.** Strings, containers and other large objects
  are passed by `const&` when the function only reads them. Pass by value only for small types,
  or for a sink that keeps its own copy (taken by value and `std::move`d, so it is never `const`).
