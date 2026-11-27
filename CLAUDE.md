# ngen_renderer

Supports both Windows and macOS: every change must build and pass its tests on both. macOS runs
Vulkan through MoltenVK, drawing into the `CAMetalLayer` from SDL's Metal view; it has been
verified on a Mac (Apple Silicon) with the LunarG SDK. Run the checks for whichever machine you are on; changes
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

Formatting is free: brace placement, spacing and similar layout choices don't matter. What matters
is that code is readable and clean. Split a function when it does several separable jobs, or when a
piece of it is worth testing on its own; don't split a function that reads as one coherent sequence
just because it is long, since a helper used once that the reader must jump to is not an
improvement. A style suggestion is worth taking when it makes the code easier to read. These two affect code quality, so follow them:

- **`const` by-value parameters in definitions.** When a definition doesn't modify a by-value
  parameter, mark it `const` there (`Error makeError(const ErrorCode code, std::string message)`).
  Leave the header declaration without it, because it doesn't affect callers.
- **Pass large objects by `const&` when only read.** Strings, containers and other large objects
  are passed by `const&` when the function only reads them. Pass by value only for small types,
  or for a sink that keeps its own copy (taken by value and `std::move`d, so it is never `const`).

## clang-tidy

`.clang-tidy` is a helper for finding what to look at, not an authority. When it points at a line,
examine the code and decide whether the suggestion is right for it before changing anything; never
apply its fixes blindly or in bulk. The build does not run it.
