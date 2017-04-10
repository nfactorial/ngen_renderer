//
// Copyright 2017 nfactorial
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

#include <vector>
#include <cstdio>
#include <cassert>

#include "shader_object.h"

bool ngen::rendering::ogl::ShaderObject::silentCompilation = false;

namespace ngen {
    namespace rendering {
        namespace ogl {
            ShaderObject::ShaderObject()
            : m_shaderId(GL_INVALID_VALUE)
            , m_type(GL_INVALID_VALUE)
            {
                //
            }

            ShaderObject::~ShaderObject() {
                if (GL_INVALID_VALUE != m_shaderId) {
                    glDeleteShader(m_shaderId);
                }
            }

            //! \brief  Destroys any data currently allocated by the object.
            void ShaderObject::dispose() {
                m_type = GL_INVALID_VALUE;

                if (GL_INVALID_VALUE != m_shaderId) {
                    glDeleteShader(m_shaderId);
                    m_shaderId = GL_INVALID_VALUE;
                }
            }

            //! \brief  Creates a vertex shader object and attempts to compile it.
            //! \param  sourceCode [in] -
            //!         Pointer to buffer containing the source code for the vertex shader.
            //! \param  sourceLength [in] -
            //!         Length (in characters) of the shader source code.
            //! \return <em>True</em> if the shader object was created successfully otherwise <em>false</em>.
            bool ShaderObject::createVertexShader(const char *sourceCode, int sourceLength) {
                return createShader(GL_VERTEX_SHADER, sourceCode, sourceLength);
            }

            //! \brief  Creates a pixel (fragment) shader object and attempts to compile it.
            //! \param  sourceCode [in] -
            //!         Pointer to buffer containing the source code for the pixel shader.
            //! \param  sourceLength [in] -
            //!         Length (in characters) of the shader source code.
            //! \return <em>True</em> if the shader object was created successfully otherwise <em>false</em>.
            bool ShaderObject::createPixelShader(const char *sourceCode, int sourceLength) {
                return createShader(GL_FRAGMENT_SHADER, sourceCode, sourceLength);
            }

            //! \brief  Compiles the shader object
            //! \param  sourceCode [in] -
            //!         Pointer to buffer containing the source code for the vertex shader.
            //! \param  sourceLength [in] -
            //!         Length (in characters) of the shader source code.
            //! \return <em>True</em> if the shader object was created successfully otherwise <em>false</em>.
            bool ShaderObject::compileShader(const char *sourceCode, int sourceLength) {
                glShaderSource(m_shaderId, 1, &sourceCode, &sourceLength);

                glCompileShader(m_shaderId);

                GLint success = 0;
                glGetShaderiv(m_shaderId, GL_COMPILE_STATUS, &success);
                if (GL_FALSE == success) {
                    if (!silentCompilation) {
                        GLint logSize = 0;
                        glGetShaderiv(m_shaderId, GL_INFO_LOG_LENGTH, &logSize);

                        assert(logSize > 0);

                        std::vector<char> data(static_cast<size_t>(logSize));
                        glGetShaderInfoLog(m_shaderId, logSize, &logSize, &data[0]);

                        // TODO: This should be a log
                        printf(&data[0]);
                    }

                    return false;
                }

                return true;
            }

            //! \brief  Helper method that performs the creation and compilation of a specified type of shader.
            //! \param  sourceCode [in] -
            //!         Pointer to buffer containing the source code for the vertex shader.
            //! \param  sourceLength [in] -
            //!         Length (in characters) of the shader source code.
            //! \return <em>True</em> if the shader was created successfully otherwise <em>false</em>.
            bool ShaderObject::createShader(GLenum shaderType, const char *sourceCode, int sourceLength) {
                if (!sourceCode) {
                    // log("Failed to create shader, no source code was provided.");
                    return false;
                }

                if (sourceLength <= 0) {
                    // log("Failed to create shader, source length was an invalid value.");
                    return false;
                }

                if (GL_INVALID_VALUE != m_shaderId) {
                    // log("Failed to create shader, shader object is already in use.");
                    return false;
                }

                m_shaderId = glCreateShader(GL_VERTEX_SHADER);
                if (!m_shaderId) {
                    //log("Failed to create shader object.");
                    return false;
                }

                if (!compileShader(sourceCode, sourceLength)) {
                    dispose();
                    return false;
                }

                m_type = shaderType;
                return true;
            }
        }
    }
}
