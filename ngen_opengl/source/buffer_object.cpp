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
                return createBufferObject(GL_ARRAY_BUFFER, dataLength, nullptr, GL_STATIC_DRAW);
            }

            //! \brief  Creates a buffer object whose content will be updated frequently.
            //! \param  dataLength [in] - The length (in bytes) of data to be contained in the buffer.
            //! \return <em>True</em> if the buffer created successfully otherwise <em>false</em>.
            bool BufferObject::createDynamic(size_t dataLength) {
                return createBufferObject(GL_ARRAY_BUFFER, dataLength, nullptr, GL_DYNAMIC_DRAW);
            }

            //! \brief Creates and initializes a buffer object of the specified type and size.
            //! \param target [in] - Specifies the target buffer object.
            //! \param dataLength [in] - The size (in bytes) of the buffer objects data store.
            //! \param usage [in] - Specifies the usage pattern of the data store.
            //! \returns True if the buffer was created successfully otherwise false.
            bool BufferObject::createBufferObject(GLenum target, size_t dataLength, const GLvoid *data, GLenum usage) {
                if (GL_INVALID_VALUE != m_bufferId) {
                    return false;
                }

                if (dataLength == 0) {
                    return false;
                }

                glGenBuffers(1, &m_bufferId);
                if (GL_INVALID_VALUE == m_bufferId) {
                    return false;
                }

                glBindBuffer(target, m_bufferId);
                glBufferData(target, dataLength, data, usage);

                return true;
            }
        }
    }
}
