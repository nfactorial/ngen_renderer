//
// Copyright 2016 nfactorial
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "buffer_object.h"
#include "gtest/gtest.h"

//! \brief  Checks the constructor initializes the object correctly.
TEST(BufferObject, Construction) {
    ngen::rendering::ogl::BufferObject buffer;

    EXPECT_EQ(GL_INVALID_VALUE, buffer.getId());
    EXPECT_EQ(0, buffer.getDataLength());
    EXPECT_FALSE(buffer.isDynamic());
}


//! \brief  Tests functionality of the enable/disable capability.
//!
//! TODO: Also need to verify the display port does not perform rendering when it's disabled.
/*
TEST(Texture, ) {
    ngen::renderer::DisplayPort displayPort;

    EXPECT_TRUE(displayPort.isEnabled());

    displayPort.setEnabled(false);
    EXPECT_FALSE(displayPort.isEnabled());

    displayPort.setEnabled(true);
    EXPECT_TRUE(displayPort.isEnabled());
}
*/