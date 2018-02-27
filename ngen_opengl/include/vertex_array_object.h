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

#ifndef NGEN_OPENGL_VERTEX_ARRAY_OBJECT_H
#define NGEN_OPENGL_VERTEX_ARRAY_OBJECT_H

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen {
    namespace rendering {
        namespace ogl {
            //! \brief Class wrapper for OpenGL Vertex Array Objects.
            //!
            //! Please see: https://www.khronos.org/opengl/wiki/Vertex_Specification
            //! for more detailed information on the actual OpenGL object this class encapsulates.
            class VertexArrayObject {
            public:
                VertexArrayObject();
                ~VertexArrayObject();

                bool create();
                void dispose();

                GLuint getId() const;

            private:
                GLuint      m_arrayId;
            };

            //! \brief  Retrieves the identifier of the opengl buffer object.
            //! \return The identifier of the opengl buffer object..
            inline GLuint VertexArrayObject::getId() const {
                return m_arrayId;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_OPENGL_VERTEX_ARRAY_OBJECT_H
