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

#include "shader_program.h"

namespace ngen {
    namespace rendering {
        namespace ogl {
            ShaderProgram::ShaderProgram()
            : m_programId(0)
            {
                //
            }

            ShaderProgram::~ShaderProgram() {
                if (m_programId) {
                    glDeleteProgram(m_programId);
                }
            }

            //! \brief Deletes any resources currently allocated by this object.
            void ShaderProgram::dispose() {
                if (m_programId) {
                    glDeleteProgram(m_programId);
                    m_programId = 0;
                }
            }

            //! \brief Attmpts to allocate resources for the shader program using the supplied data.
            //! \param vertexShader - The vertex shader to be used by the program when rendering.
            //! \param pixelShader - The pixel shader to be used by the program when rendering.
            //! \returns True if the shader program was successfully created, otherwise false.
            bool ShaderProgram::create(ShaderObject *vertexShader, ShaderObject *pixelShader) {
                if (!vertexShader) {
                    // log("No vertex shader specified.");
                    return false;
                }

                if (!pixelShader) {
                    // log("No pixel shader specified.");
                    return false;
                }

                if (m_programId) {
                    // log("ShaderProgram already exists.");
                    return false;
                }

                m_programId = glCreateProgram();
                return (m_programId != 0);
            }
        }
    }
}
