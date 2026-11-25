// Spec: platform. Behaviour changes go through it.
#include <ngen/platform/fake_window.h>

#include <gtest/gtest.h>

using namespace ngen::platform;

namespace
{
    int firstTarget = 0;
    int secondTarget = 0;

    const NativeWindow handlesA = Win32Handles{&firstTarget, &firstTarget};
    const NativeWindow handlesB = Win32Handles{&secondTarget, &secondTarget};

    std::vector<WindowEvent> poll(Window &window)
    {
        std::vector<WindowEvent> events;
        window.pollEvents(events);
        return events;
    }
} // namespace

TEST(FakeWindow, ResizeToZeroThenNewSizeDeliversTwoEventsInOrder)
{
    FakeWindow window(handlesA, {800, 600});
    EXPECT_EQ(window.pixelSize(), (PixelSize{800, 600}));

    window.resize({0, 0});
    window.resize({1024, 768});

    const std::vector<WindowEvent> events = poll(window);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0], (WindowEvent{WindowEvent::Kind::Resized, {0, 0}}));
    EXPECT_EQ(events[1], (WindowEvent{WindowEvent::Kind::Resized, {1024, 768}}));
    EXPECT_EQ(window.pixelSize(), (PixelSize{1024, 768}));
}

TEST(FakeWindow, SuspendLosesSurfaceAndResumeRestoresNewHandles)
{
    FakeWindow window(handlesA, {800, 600});
    ASSERT_TRUE(window.nativeWindow().has_value());
    EXPECT_EQ(*window.nativeWindow(), handlesA);

    window.suspend();
    const auto lost = window.nativeWindow();
    ASSERT_FALSE(lost.has_value());
    EXPECT_EQ(lost.error().code, ngen::ErrorCode::SurfaceLost);

    window.resume(handlesB);
    const auto resumed = window.nativeWindow();
    ASSERT_TRUE(resumed.has_value());
    EXPECT_EQ(*resumed, handlesB);

    const std::vector<WindowEvent> events = poll(window);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0], (WindowEvent{WindowEvent::Kind::Suspended, {0, 0}}));
    EXPECT_EQ(events[1], (WindowEvent{WindowEvent::Kind::Resumed, {800, 600}}));
}

TEST(FakeWindow, ResumeCarriesTheCurrentSize)
{
    FakeWindow window(handlesA, {800, 600});
    window.suspend();
    window.resize({640, 480});
    window.resume(handlesB);

    const std::vector<WindowEvent> events = poll(window);
    ASSERT_EQ(events.size(), 3u);
    EXPECT_EQ(events[2], (WindowEvent{WindowEvent::Kind::Resumed, {640, 480}}));
}

TEST(FakeWindow, RequestCloseQueuesCloseRequested)
{
    FakeWindow window(handlesA, {800, 600});
    window.requestClose();

    const std::vector<WindowEvent> events = poll(window);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0], (WindowEvent{WindowEvent::Kind::CloseRequested, {0, 0}}));
}

TEST(FakeWindow, SecondPollWithNothingNewAppendsNothing)
{
    FakeWindow window(handlesA, {800, 600});
    window.resize({1, 1});
    EXPECT_EQ(poll(window).size(), 1u);

    std::vector<WindowEvent> events{{WindowEvent::Kind::CloseRequested, {0, 0}}};
    window.pollEvents(events);
    EXPECT_EQ(events.size(), 1u);
}
