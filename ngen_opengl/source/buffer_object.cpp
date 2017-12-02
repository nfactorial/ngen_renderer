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

#include "buffer_object.h"

namespace ngen {
    namespace rendering {
        namespace ogl {
            BufferObject::BufferObject()
            : m_bufferId(GL_INVALID_VALUE)
            , m_dataLength(0)
            , m_isDynamic(false)
            {
                //
            }

            BufferObject::~BufferObject() {
                if (GL_INVALID_VALUE != m_bufferId) {
                    glDeleteBuffers(1, &m_bufferId);
                }
            }

            //! \brief Deletes any resources currently allocated by this object.
            void BufferObject::dispose() {
                if (GL_INVALID_VALUE != m_bufferId) {
                    glDeleteBuffers(1, &m_bufferId);
                    m_bufferId = GL_INVALID_VALUE;
                }
            }

            //! \brief  Creates a buffer object whose content will not be updated frequently.
            //! \param  dataLength [in] - The length (in bytes) of data to be contained in the buffer.
            //! \return <em>True</em> if the buffer created successfully otherwise <em>false</em>.
            bool BufferObject::createStatic(size_t dataLength) {
                return false;
            }

            //! \brief  Creates a buffer object whose content will be updated frequently.
            //! \param  dataLength [in] - The length (in bytes) of data to be contained in the buffer.
            //! \return <em>True</em> if the buffer created successfully otherwise <em>false</em>.
            bool BufferObject::createDynamic(size_t dataLength) {
                return false;
            }
        }
    }
}
