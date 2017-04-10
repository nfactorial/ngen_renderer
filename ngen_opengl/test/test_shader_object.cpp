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

#include "shader_object.h"
#include "gtest/gtest.h"

// Here we define a few in-memory shader sources for testing the behavior of the shader object. We can extend
// the sources in the future if we find particular behaviors we wish to verify.

static const char TEST_VALID_VERTEX_SHADER[] =
"uniform mat4 modelViewMatrix;\n" \
"uniform mat4 projectionMatrix;\n" \
"attribute vec4 vertexPosition;\n" \
"void main() {\n" \
"    gl_Position = projectionMatrix * modelViewMatrix * vertexPosition;\n" \
"}\n";

static const char TEST_INVALID_VERTEX_SHADER[] =
"attribute vec4 vertexPosition;\n" \
"void main() {\n" \
"    gl_Position = projectionMatrix * modelViewMatrix * vertexPosition;\n" \
"}\n";

static const char TEST_VALID_FRAGMENT_SHADER[] =
"void main() {\n" \
"    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n" \
"}\n";

static const char TEST_INVALID_FRAGMENT_SHADER[] =
"void main() {\n" \
"    gl_FragColor = fragment_color;\n" \
"}\n";

//! \brief  Checks the constructor initializes the object correctly.
TEST(ShaderObject, Construction) {
    ngen::rendering::ogl::ShaderObject shader;

    EXPECT_EQ(GL_INVALID_VALUE, shader.getId());
}

//! \brief  Check the createShader method behaves as expected.
TEST(ShaderObject, createVertexShader) {
    ngen::rendering::ogl::ShaderObject shader;

    // Prevent debug output being sent to the console window
    ngen::rendering::ogl::ShaderObject::silentCompilation = true;

    EXPECT_FALSE(shader.createVertexShader(nullptr, 0));
    EXPECT_FALSE(shader.createVertexShader(nullptr, sizeof(TEST_VALID_VERTEX_SHADER)));
    EXPECT_FALSE(shader.createVertexShader(TEST_VALID_VERTEX_SHADER, 0));

    EXPECT_FALSE(shader.createVertexShader(TEST_INVALID_VERTEX_SHADER, sizeof(TEST_INVALID_VERTEX_SHADER)));
    shader.dispose();

    EXPECT_TRUE(shader.createVertexShader(TEST_VALID_VERTEX_SHADER, sizeof(TEST_VALID_VERTEX_SHADER)));
}

//! \brief  Check the createShader method behaves as expected.
TEST(ShaderObject, createPixelShader) {
    ngen::rendering::ogl::ShaderObject shader;

    // Prevent debug output being sent to the console window
    ngen::rendering::ogl::ShaderObject::silentCompilation = true;

    EXPECT_FALSE(shader.createPixelShader(nullptr, 0));
    EXPECT_FALSE(shader.createPixelShader(nullptr, sizeof(TEST_VALID_FRAGMENT_SHADER)));
    EXPECT_FALSE(shader.createPixelShader(TEST_VALID_FRAGMENT_SHADER, 0));

    EXPECT_FALSE(shader.createPixelShader(TEST_INVALID_FRAGMENT_SHADER, sizeof(TEST_INVALID_FRAGMENT_SHADER)));
    shader.dispose();

    EXPECT_TRUE(shader.createPixelShader(TEST_VALID_FRAGMENT_SHADER, sizeof(TEST_VALID_FRAGMENT_SHADER)));
}
