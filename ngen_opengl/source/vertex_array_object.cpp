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

#include "vertex_array_object.h"

namespace ngen {
    namespace rendering {
        namespace ogl {
            VertexArrayObject::VertexArrayObject()
            : m_arrayId(GL_INVALID_VALUE)
            {
                //
            }

            VertexArrayObject::~VertexArrayObject() {
                if (GL_INVALID_VALUE != m_arrayId) {
                    glDeleteVertexArrays(1, &m_arrayId);
                }
            }

            //! \brief Deletes any resources currently allocated by this object.
            void VertexArrayObject::dispose() {
                if (GL_INVALID_VALUE != m_arrayId) {
                    glDeleteVertexArrays(1, &m_arrayId);
                    m_arrayId = GL_INVALID_VALUE;
                }
            }

            //! \brief Attempts to create a new OpenGL vertex array object.
            //! \returns <em>True</em> if the vertex array was created successfully otherwise <em>false</em>.
            bool VertexArrayObject::create() {
                if (GL_INVALID_VALUE != m_arrayId) {
                    return false;
                }

                glGenVertexArrays(1, &m_arrayId);
                return (GL_INVALID_VALUE != m_arrayId);
            }
        }
    }
}
