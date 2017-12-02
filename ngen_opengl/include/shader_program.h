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

#ifndef NGEN_OPENGL_BUFFER_OBJECT_H
#define NGEN_OPENGL_BUFFER_OBJECT_H

////////////////////////////////////////////////////////////////////////////

#include "platform.h"

////////////////////////////////////////////////////////////////////////////

namespace ngen {
    namespace rendering {
        namespace ogl {
            class ShaderObject;

            //! \brief Wrapper around OpenGL shader program object. A shader program consists of a vertex shader
            //! and an (optional) pixel (fragment) shader.
            class ShaderProgram {
            public:
                ShaderProgram();
                ~ShaderProgram();

                void dispose();

                bool create(ShaderObject *vertexShader, ShaderObject *pixelShader);

                GLuint getId() const;

            private:
                GLuint      m_programId;
            };

            //! \brief  Retrieves the identifier of the opengl buffer object.
            //! \return The identifier of the opengl buffer object..
            inline GLuint ShaderProgram::getId() const {
                return m_programId;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_OPENGL_BUFFER_OBJECT_H
