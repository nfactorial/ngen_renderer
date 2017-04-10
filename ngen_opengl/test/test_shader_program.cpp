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

#include <stdio.h>

#include "shader_program.h"
#include "gtest/gtest.h"

//! \brief  Ensures the constructor behaves as expected.
TEST(ShaderProgram, Construction) {
    ngen::rendering::ogl::ShaderProgram program;

    EXPECT_EQ(0, program.getId());
}


//! \brief  Checks the object behaves correctly during creation of a new program.
TEST(ShaderProgram, create) {
    ngen::rendering::ogl::ShaderProgram program;

    EXPECT_TRUE(program.create());
}
