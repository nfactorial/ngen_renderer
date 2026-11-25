# ngen_renderer

Supports both Windows and macOS: every change must build and pass its tests on both.

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
