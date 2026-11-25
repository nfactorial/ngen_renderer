// Spec: platform. Behaviour changes go through it.
#include <ngen/platform/sdl_window.h>

#include <gtest/gtest.h>

using namespace ngen::platform;

namespace
{
    // Initialises SDL's video subsystem around each test. Off Apple the dummy driver is used, so
    // the tests need no display; Apple needs the real driver for its Metal view.
    class SdlWindowTest : public testing::Test
    {
    protected:
        void SetUp() override
        {
#if !defined(__APPLE__)
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
#endif
            ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO)) << SDL_GetError();
        }

        void TearDown() override
        {
            SDL_Quit();
        }

        std::unique_ptr<SdlWindow> makeWindow()
        {
            auto window = SdlWindow::create({"test", 320, 240});
            EXPECT_TRUE(window.has_value());
            return window.has_value() ? std::move(*window) : nullptr;
        }

        static std::vector<WindowEvent> poll(SdlWindow &window)
        {
            std::vector<WindowEvent> events;
            window.pollEvents(events);
            return events;
        }
    };

    SDL_Event windowEvent(Uint32 type, SDL_WindowID id, int data1 = 0, int data2 = 0)
    {
        SDL_Event event{};
        event.type = type;
        event.window.windowID = id;
        event.window.data1 = data1;
        event.window.data2 = data2;
        return event;
    }

    SDL_Event plainEvent(Uint32 type)
    {
        SDL_Event event{};
        event.type = type;
        return event;
    }
} // namespace

TEST_F(SdlWindowTest, ZeroWidthOrHeightIsInvalidArgument)
{
    const auto noWidth = SdlWindow::create({"test", 0, 240});
    ASSERT_FALSE(noWidth.has_value());
    EXPECT_EQ(noWidth.error().code, ngen::ErrorCode::InvalidArgument);

    const auto noHeight = SdlWindow::create({"test", 320, 0});
    ASSERT_FALSE(noHeight.has_value());
    EXPECT_EQ(noHeight.error().code, ngen::ErrorCode::InvalidArgument);
}

TEST_F(SdlWindowTest, EmptyTitleIsInvalidArgument)
{
    const auto window = SdlWindow::create({"", 320, 240});
    ASSERT_FALSE(window.has_value());
    EXPECT_EQ(window.error().code, ngen::ErrorCode::InvalidArgument);
}

TEST(SdlWindowCreate, SdlFailureIsInternalWithSdlText)
{
    // SDL is not initialised and its only allowed video driver does not exist, so SDL_CreateWindow fails.
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "no_such_driver");
    SDL_ClearError();
    const auto window = SdlWindow::create({"test", 320, 240});
    SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
    ASSERT_FALSE(window.has_value());
    EXPECT_EQ(window.error().code, ngen::ErrorCode::Internal);
    EXPECT_NE(window.error().message.find(SDL_GetError()), std::string::npos);
}

TEST_F(SdlWindowTest, CreateMakesAHighPixelDensityResizableWindow)
{
    const auto window = makeWindow();
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->sdlWindow(), nullptr);

    const SDL_WindowFlags flags = SDL_GetWindowFlags(window->sdlWindow());
    EXPECT_TRUE(flags & SDL_WINDOW_HIGH_PIXEL_DENSITY);
    EXPECT_TRUE(flags & SDL_WINDOW_RESIZABLE);
#if defined(__APPLE__)
    EXPECT_TRUE(flags & SDL_WINDOW_METAL);
#else
    EXPECT_FALSE(flags & SDL_WINDOW_METAL);
#endif
}

TEST_F(SdlWindowTest, NonResizableDescriptionOmitsTheResizableFlag)
{
    auto window = SdlWindow::create({"test", 320, 240, false});
    ASSERT_TRUE(window.has_value());
    EXPECT_FALSE(SDL_GetWindowFlags((*window)->sdlWindow()) & SDL_WINDOW_RESIZABLE);
}

TEST_F(SdlWindowTest, PixelSizeChangedBecomesResized)
{
    const auto window = makeWindow();
    ASSERT_NE(window, nullptr);

    window->handleEvent(windowEvent(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED, SDL_GetWindowID(window->sdlWindow()), 1024, 768));
    window->handleEvent(windowEvent(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED, SDL_GetWindowID(window->sdlWindow()), 0, 0));

    const std::vector<WindowEvent> events = poll(*window);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0], (WindowEvent{WindowEvent::Kind::Resized, {1024, 768}}));
    EXPECT_EQ(events[1], (WindowEvent{WindowEvent::Kind::Resized, {0, 0}}));
}

TEST_F(SdlWindowTest, MinimizedBecomesResizedToZero)
{
    const auto window = makeWindow();
    ASSERT_NE(window, nullptr);
    const SDL_WindowID id = SDL_GetWindowID(window->sdlWindow());

    window->handleEvent(windowEvent(SDL_EVENT_WINDOW_MINIMIZED, id));
    const std::vector<WindowEvent> events = poll(*window);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0], (WindowEvent{WindowEvent::Kind::Resized, {0, 0}}));
}

TEST_F(SdlWindowTest, RestoredBecomesResizedToCurrentPixelSize)
{
    const auto window = makeWindow();
    ASSERT_NE(window, nullptr);
    const SDL_WindowID id = SDL_GetWindowID(window->sdlWindow());

    ASSERT_NE(window->pixelSize().width, 0u);

    window->handleEvent(windowEvent(SDL_EVENT_WINDOW_RESTORED, id));
    const std::vector<WindowEvent> events = poll(*window);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0], (WindowEvent{WindowEvent::Kind::Resized, window->pixelSize()}));
}

TEST_F(SdlWindowTest, CloseRequestedAndQuitBecomeCloseRequested)
{
    const auto window = makeWindow();
    ASSERT_NE(window, nullptr);

    window->handleEvent(windowEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED, SDL_GetWindowID(window->sdlWindow())));
    window->handleEvent(plainEvent(SDL_EVENT_QUIT));

    const std::vector<WindowEvent> events = poll(*window);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0], (WindowEvent{WindowEvent::Kind::CloseRequested, {0, 0}}));
    EXPECT_EQ(events[1], (WindowEvent{WindowEvent::Kind::CloseRequested, {0, 0}}));
}

TEST_F(SdlWindowTest, BackgroundAndForegroundBecomeSuspendedAndResumed)
{
    const auto window = makeWindow();
    ASSERT_NE(window, nullptr);

    window->handleEvent(plainEvent(SDL_EVENT_WILL_ENTER_BACKGROUND));
    const auto lost = window->nativeWindow();
    ASSERT_FALSE(lost.has_value());
    EXPECT_EQ(lost.error().code, ngen::ErrorCode::SurfaceLost);

    window->handleEvent(plainEvent(SDL_EVENT_DID_ENTER_FOREGROUND));

    const std::vector<WindowEvent> events = poll(*window);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0], (WindowEvent{WindowEvent::Kind::Suspended, {0, 0}}));
    EXPECT_EQ(events[1], (WindowEvent{WindowEvent::Kind::Resumed, window->pixelSize()}));
    EXPECT_NE(window->pixelSize().width, 0u);
    // The dummy video driver has no native handles, so only "not SurfaceLost" holds everywhere.
    const auto resumed = window->nativeWindow();
    EXPECT_TRUE(resumed.has_value() || resumed.error().code != ngen::ErrorCode::SurfaceLost);
}

TEST_F(SdlWindowTest, EventsForOtherWindowsAreIgnored)
{
    const auto window = makeWindow();
    ASSERT_NE(window, nullptr);
    const SDL_WindowID other = SDL_GetWindowID(window->sdlWindow()) + 1000;

    window->handleEvent(windowEvent(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED, other, 10, 10));
    window->handleEvent(windowEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED, other));
    window->handleEvent(windowEvent(SDL_EVENT_WINDOW_MINIMIZED, other));
    window->handleEvent(windowEvent(SDL_EVENT_WINDOW_RESTORED, other));
    window->handleEvent(plainEvent(SDL_EVENT_KEY_DOWN));

    EXPECT_TRUE(poll(*window).empty());
}

TEST_F(SdlWindowTest, SecondPollWithNothingNewAppendsNothing)
{
    const auto window = makeWindow();
    ASSERT_NE(window, nullptr);
    window->handleEvent(plainEvent(SDL_EVENT_QUIT));
    EXPECT_EQ(poll(*window).size(), 1u);
    EXPECT_TRUE(poll(*window).empty());
}

TEST_F(SdlWindowTest, DestroyingTheWindowDestroysTheSdlWindow)
{
    auto window = makeWindow();
    ASSERT_NE(window, nullptr);
    const SDL_WindowID id = SDL_GetWindowID(window->sdlWindow());
    ASSERT_NE(SDL_GetWindowFromID(id), nullptr);

    window.reset();
    EXPECT_EQ(SDL_GetWindowFromID(id), nullptr);
}

#if defined(__APPLE__)
TEST_F(SdlWindowTest, AppleNativeWindowHoldsAMetalLayer)
{
    const auto window = makeWindow();
    ASSERT_NE(window, nullptr);
    const auto handles = window->nativeWindow();
    ASSERT_TRUE(handles.has_value());
    ASSERT_TRUE(std::holds_alternative<AppleHandles>(*handles));
    EXPECT_NE(std::get<AppleHandles>(*handles).metalLayer, nullptr);
}
#endif

#if defined(_WIN32)
#include <windows.h>

// These use SDL's real video driver (the dummy driver has no HWND).
class SdlWindowWin32 : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO)) << SDL_GetError();
    }

    void TearDown() override
    {
        SDL_Quit();
    }
};

// On a hidden window.
TEST_F(SdlWindowWin32, NativeWindowHoldsAValidWin32Handle)
{
    {
        auto created = SdlWindow::create({"test", 320, 240});
        ASSERT_TRUE(created.has_value());
        SdlWindow &window = **created;
        SDL_HideWindow(window.sdlWindow());

        const auto checkHandles = [&window]
        {
            const auto handles = window.nativeWindow();
            ASSERT_TRUE(handles.has_value());
            ASSERT_TRUE(std::holds_alternative<Win32Handles>(*handles));
            const Win32Handles win32 = std::get<Win32Handles>(*handles);
            EXPECT_NE(win32.hwnd, nullptr);
            EXPECT_NE(win32.hinstance, nullptr);
            EXPECT_TRUE(IsWindow(static_cast<HWND>(win32.hwnd)));
        };

        checkHandles();

        SDL_Event event{};
        event.type = SDL_EVENT_WILL_ENTER_BACKGROUND;
        window.handleEvent(event);
        event.type = SDL_EVENT_DID_ENTER_FOREGROUND;
        window.handleEvent(event);
        checkHandles();
    }
}

TEST_F(SdlWindowWin32, PixelSizeIsZeroWhileMinimised)
{
    {
        auto created = SdlWindow::create({"test", 320, 240});
        ASSERT_TRUE(created.has_value());
        SdlWindow &window = **created;
        ASSERT_NE(window.pixelSize().width, 0u);

        SDL_MinimizeWindow(window.sdlWindow());
        SDL_SyncWindow(window.sdlWindow());
        ASSERT_TRUE(SDL_GetWindowFlags(window.sdlWindow()) & SDL_WINDOW_MINIMIZED);
        EXPECT_EQ(window.pixelSize(), (PixelSize{0, 0}));

        SDL_RestoreWindow(window.sdlWindow());
        SDL_SyncWindow(window.sdlWindow());
        EXPECT_NE(window.pixelSize().width, 0u);
    }
}
#endif
