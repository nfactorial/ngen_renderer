# Platform — Spec

> - **Template:** v1 · **ID:** platform
> - **Status:** built
> - **Blocked by:** none
> - **Background (not needed to build):** Feature: [ngen](README.md) · Design: [renderer](../design/renderer.md) · Technical: [renderer](../technical/renderer.md) · Inputs: none

## Goal

`ngen_platform` gives the rest of ngen a window to present to without knowing how the window
was made. It reports the window's native handles and pixel size, and the events that change
them: resizes, suspend and resume (on mobile the surface goes away while suspended), and close
requests. It ships one implementation, built on SDL3, and a fake window for tests. It also
holds `ngen::Error`, the error type every ngen module returns.

## Scope

**In:**
- `ngen::Error`, the shared error value.
- The `Window` interface, the `NativeWindow` and `PixelSize` values, and `WindowEvent`.
- `SdlWindow`, the SDL3 implementation, for Windows, macOS, Android and iOS.
- `FakeWindow`, a scriptable implementation for tests.
- The project-wide build settings the first ngen modules need: C++23, the Visual Studio 2022
  preset, SDL3, and GoogleTest with CTest.

**Out (non-goals):**
- Input (keyboard, mouse, touch, controllers): games use SDL directly.
- Creating graphics surfaces (`VkSurfaceKHR` and the like): a graphics backend does that from
  the native handles.
- More than one window.

## Constraints and key decisions

- No graphics API type or header appears in `ngen_platform`'s public headers, because the same
  native handles must serve Vulkan and a future D3D12 backend.
- Functions that can fail return `std::expected<T, ngen::Error>`, marked `[[nodiscard]]`. ngen
  code throws no exceptions.
- `ngen_platform` prints nothing; failures are returned as errors.
- Every public type is in namespace `ngen::platform`, except `ngen::Error`.
- Public headers live under `ngen_platform/include/ngen/platform/` (and
  `ngen_platform/include/ngen/error.h`), and are included as `<ngen/platform/…>`.
- The CMake target is `ngen_platform`, with the alias `ngen::platform`.
- The whole project builds as C++23 (`CMAKE_CXX_STANDARD 23`, required, no extensions).
- The Windows configure preset uses the "Visual Studio 17 2022" generator, x64; the macOS preset
  uses Ninja, Debug. Both platforms are supported.
- SDL3 is found as an installed CMake package, or else fetched and built statically, as SDL2 is
  today. The fetch is pinned to `release-3.2.30`, with SDL's test library off. SDL2 stays available to the
  code that still uses it until that code is removed, and the two share SDL's static/shared
  build options.
- Tests use GoogleTest and GoogleMock, fetched with `FetchContent` (tag `v1.17.0`), registered
  with `gtest_discover_tests`, and run by `ctest`. A test target is built only when the CMake
  option `NGEN_BUILD_TESTS` is on (default ON), beside `NGEN_BUILD_EXAMPLE`. The test target is
  `ngen_platform_tests`.

## Modules and interfaces

### Error  (ngen_platform/include/ngen/error.h)

The value every ngen operation returns on failure: what kind of failure, and a message saying
what failed and why.

**Interface:**
- `enum class ngen::ErrorCode { InvalidArgument, Unsupported, OutOfMemory, NotFound, SurfaceLost, DeviceLost, Internal }`
- `struct ngen::Error { ErrorCode code; std::string message; }`
- `ngen::Error ngen::makeError(ErrorCode, std::string message)`

**Requirements:**
- `message` is never empty: `makeError` with an empty message gives "unspecified <Code> error".

### Window  (ngen_platform/include/ngen/platform/window.h)

The interface a graphics backend and a game use to present to a window and follow its changes.

**Interface:**
- `struct PixelSize { uint32_t width; uint32_t height; }`: the drawable size in pixels (not
  points); either may be 0 while minimised.
- `struct Win32Handles { void *hwnd; void *hinstance; }`, `struct AndroidHandles { void *nativeWindow; }`
  (an `ANativeWindow*`), `struct AppleHandles { void *metalLayer; }` (a `CAMetalLayer*`).
- `using NativeWindow = std::variant<Win32Handles, AndroidHandles, AppleHandles>`
- `struct WindowEvent { enum class Kind { Resized, Suspended, Resumed, CloseRequested } kind; PixelSize size; }`:
  `size` holds the new pixel size for `Resized` and `Resumed`, and `{0, 0}` otherwise.
- `PixelSize`, `Win32Handles`, `AndroidHandles`, `AppleHandles` and `WindowEvent` have a defaulted
  `operator==`, so tests can compare them.
- `class Window` (abstract):
  - `std::expected<NativeWindow, ngen::Error> nativeWindow() const`: the current native handles;
    an error with code `SurfaceLost` while suspended.
  - `PixelSize pixelSize() const`: the current drawable size.
  - `void pollEvents(std::vector<WindowEvent> &events)`: appends every event since the last
    call, in the order they happened, and returns at once when there are none.
  - `virtual ~Window()`
- `SdlWindow` and `FakeWindow` derive from `Window` directly, and nothing derives from them.

**Requirements:**
- After a `Suspended` event and until the next `Resumed`, `nativeWindow()` returns `SurfaceLost`,
  and the handles from before the suspension must be treated as invalid.
- After `Resumed`, `nativeWindow()` returns the handles valid from then on, which may differ from
  those before the suspension.
- A `Resized` event is delivered whenever the drawable size changes, including to and from 0.

### SdlWindow  (ngen_platform/include/ngen/platform/sdl_window.h)

The SDL3 implementation of `Window`. It owns the SDL window; SDL itself stays initialised by
the application.

**Interface:**
- `struct WindowDescription { std::string title; uint32_t width; uint32_t height; bool resizable = true; }`
- `static std::expected<std::unique_ptr<SdlWindow>, ngen::Error> SdlWindow::create(const WindowDescription&)`:
  creates the window; requires `SDL_Init(SDL_INIT_VIDEO)` to have succeeded.
- `SDL_Window *SdlWindow::sdlWindow() const`: the SDL window, for games that use SDL directly.
- `void SdlWindow::handleEvent(const SDL_Event&)`: the application passes every SDL event it
  polls; the window turns those about itself into `WindowEvent`s for `pollEvents`.

**Requirements:**
- `create` with a width or height of 0, or an empty title, returns `InvalidArgument`.
- `create` returns `Internal` with SDL's error text when SDL fails to create the window.
- The window is created with the flag each platform needs for its native handle (on Apple, a
  Metal view; elsewhere none), and is high-pixel-density aware.
- Native handles come from the SDL3 window properties (`SDL_PROP_WINDOW_WIN32_HWND_POINTER` and
  `SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER`, `SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER`) and, on
  Apple, the `CAMetalLayer` of a Metal view made with `SDL_Metal_CreateView`. A missing Win32
  handle gives `Internal`, a missing Android window gives `SurfaceLost`, and any other platform
  gives `Unsupported`.
- SDL's `SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED` becomes `Resized`;
  `SDL_EVENT_WINDOW_MINIMIZED` becomes `Resized` with size `{0, 0}` (some backends, Windows among
  them, send no size change on minimise); `SDL_EVENT_WINDOW_RESTORED` becomes `Resized` with the
  current pixel size; `SDL_EVENT_WILL_ENTER_BACKGROUND` becomes `Suspended`; `SDL_EVENT_DID_ENTER_FOREGROUND` becomes
  `Resumed`; `SDL_EVENT_WINDOW_CLOSE_REQUESTED` and `SDL_EVENT_QUIT` become `CloseRequested`.
  Window events for other windows are ignored; the background and foreground events carry no
  window and always apply. The window tracks suspension itself, and `Resumed` carries the pixel
  size at that moment.
- `pixelSize()` returns `{0, 0}` while `SDL_WINDOW_MINIMIZED` is set.
- Destroying the `SdlWindow` destroys its Metal view (if any) and its SDL window.

### FakeWindow  (ngen_platform/include/ngen/platform/fake_window.h)

A `Window` whose state and events a test sets directly.

**Interface:**
- `FakeWindow(NativeWindow handles, PixelSize size)`
- `void resize(PixelSize)`: sets the size and queues a `Resized` event.
- `void suspend()`: queues `Suspended`; `nativeWindow()` returns `SurfaceLost` until `resume`.
- `void resume(NativeWindow handles)`: sets the handles and queues `Resumed` with the current size.
- `void requestClose()`: queues `CloseRequested`.

**Acceptance:**
- A `FakeWindow` of size 800 × 600, after `resize({0, 0})` then `resize({1024, 768})`, delivers two
  `Resized` events in that order through one `pollEvents` call, and `pixelSize()` is 1024 × 768.
- After `suspend()`, `nativeWindow()` returns `SurfaceLost`; after `resume(h)`, it returns `h`, and
  `pollEvents` delivers `Suspended` then `Resumed` carrying the current size.
- A second `pollEvents` call with nothing new appends nothing.
