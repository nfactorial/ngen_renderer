// Spec: platform. Behaviour changes go through it.
#include <ngen/error.h>

#include <gtest/gtest.h>

TEST(Error, MakeErrorKeepsCodeAndMessage)
{
    const ngen::Error error = ngen::makeError(ngen::ErrorCode::NotFound, "no such thing");
    EXPECT_EQ(error.code, ngen::ErrorCode::NotFound);
    EXPECT_EQ(error.message, "no such thing");
}

TEST(Error, MessageIsNeverEmpty)
{
    const ngen::Error error = ngen::makeError(ngen::ErrorCode::Internal, "");
    EXPECT_EQ(error.code, ngen::ErrorCode::Internal);
    EXPECT_FALSE(error.message.empty());
}
