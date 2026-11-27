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

These describe how the existing code is written. Follow them so new code reads as if one person
wrote all of it; when a rule doesn't fit a case, say so rather than quietly working around it.

### Readability

Formatting is free: brace placement, spacing and similar layout choices don't matter. What matters
is that code is readable and clean. Split a function when it does several separable jobs, or when a
piece of it is worth testing on its own; don't split a function that reads as one coherent sequence
just because it is long, since a helper used once that the reader must jump to is not an
improvement. A style suggestion is worth taking when it makes the code easier to read.

Add no abstraction, option or layer that the current specs don't need. Check what an invariant
already guarantees instead of re-checking it defensively.

### Errors

- Operations that can fail return `std::expected<T, ngen::Error>` (`std::expected<void, ngen::Error>`
  when there is no value), built with `ngen::makeError` and an `ErrorCode`. Every such function is
  `[[nodiscard]]`.
- ngen does not throw. Code is written so a throw from the standard library (such as allocation
  failure) leaves no leak, but nothing catches or converts it.
- An error message says what failed and why, naming the call (`"beginFrame: a frame is already
  being recorded"`). Vulkan failures name the Vulkan function and its result.
- Misuse the API defines as harmless (destroying a null or stale handle) is ignored, as the spec
  says. ngen prints nothing itself: diagnostics go to the device's message sink or are returned.

### Ownership and lifetime

- Every resource is owned by exactly one object whose destructor releases it (RAII). A class
  that owns a resource is move-only: it deletes its copy operations and has `noexcept` move
  operations that leave the source empty (`std::exchange` with a null value).
- Types that own nothing follow the rule of zero: no user-declared copy, move or destructor. This
  includes public concrete classes (such as `Renderer`); the deleted copies below are for
  interfaces only.
- Ownership is `std::unique_ptr` or a value member; non-owning access is a reference, or a raw
  pointer when it may be null. No `shared_ptr` unless ownership is truly shared, and no bare `new`
  outside a private-constructor factory that wraps it at once in a `unique_ptr`.
- Objects that can fail to construct have a `static create` returning `std::expected`, and a
  private constructor.
- Resources the caller names are handles (`Handle<Tag>` from a `HandlePool`), never pointers, so a
  stale handle is detected rather than dangling.

### Interfaces

- Public headers live under `include/ngen/...`; implementation headers stay in `source/`. Backend
  types (Vulkan, VMA, SDL) never appear in a public header: the public class forwards to an
  implementation class held by `unique_ptr` (`VulkanDevice` → `DeviceImpl`).
- Interfaces are abstract classes with deleted copies and a virtual destructor; implementations
  are `final`. Single-argument constructors are `explicit`.
- Enums are `enum class`. Plain data (descriptions, handles) are aggregates with default member
  initialisers, and use a defaulted `operator==` where they're compared.
- Templates are used where they remove real duplication or add type safety (as `Handle<Tag>`
  does); they are not used to make code generic in advance.

### Parameters and `const`

- **`const` by-value parameters in definitions.** When a definition doesn't modify a by-value
  parameter, mark it `const` there (`Error makeError(const ErrorCode code, std::string message)`).
  Leave the header declaration without it, because it doesn't affect callers.
- **Pass large objects by `const&` when only read.** Strings, containers and other large objects
  are passed by `const&` when the function only reads them. Pass by value only for small types,
  or for a sink that keeps its own copy (taken by value and `std::move`d, so it is never `const`).
  Contiguous read-only input is a `std::span<const T>`.
- Locals that don't change are `const`, unless they are moved from (a `const` object can't be). Member functions that don't change state are `const`.

### Portability

ngen targets Windows, macOS, Android and iOS. A standard library feature is used only when every
target's toolchain has it: MSVC, Apple clang (within the iOS deployment target) and the Android
NDK's clang and libc++, which usually lag the others. A feature that compiles on one machine is
not enough; when unsure, use the older equivalent.

Decisions:

- **No C++23 range algorithms yet (2026-11).** `std::ranges::iota` reached libc++ only in 2025,
  and it is unconfirmed that the current NDK ships it, so `std::iota` is used instead, and
  clang-tidy's `modernize-use-ranges` is off. C++20 range algorithms (`std::ranges::copy`,
  `std::ranges::stable_sort`) are fine. Revisit once the Android build exists and its toolchain
  can be checked.

### Comments

Comments say why, or state a contract the code can't (what a function guarantees, what a caller
must not do, why an ordering matters). They don't narrate what the next line does. Each source
file starts with `// Spec: <id>. Behaviour changes go through it.`, naming the spec it was built
from.

### Tests

Tests use GoogleTest, named `Suite.WhatHappensUnderWhichCondition`. Logic is tested through the
fake backend (`FakeDevice`, `FakeWindow`) or through pure functions taking plain inputs (as
`choosePhysicalDevice` does), so it runs without a GPU; tests that need a real device are labelled
`gpu`. A test asserts the behaviour the spec states, including, for a test that uses a device, that
no warning or error reaches the message sink.

## clang-tidy

`.clang-tidy` is a helper for finding what to look at, not an authority. When it points at a line,
examine the code and decide whether the suggestion is right for it before changing anything; never
apply its fixes blindly or in bulk. The build does not run it.
