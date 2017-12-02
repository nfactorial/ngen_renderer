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

#include <stddef.h>

#include "platform.h"

////////////////////////////////////////////////////////////////////////////

namespace ngen {
    namespace rendering {
        namespace ogl {
            //! \brief Class wrapper for OpenGL Buffer Objects.
            //!
            //! Please see: https://www.khronos.org/opengl/wiki/Buffer_Object
            //! for more detailed information on the actual OpenGL object this class encapsulates.
            class BufferObject {
            public:
                BufferObject();
                ~BufferObject();

                void dispose();

                GLuint getId() const;

                bool isDynamic() const;
                size_t getDataLength() const;

                bool createStatic(size_t dataLength);
                bool createDynamic(size_t dataLength);

            private:
                GLuint      m_bufferId;
                size_t      m_dataLength;
                bool        m_isDynamic;
            };

            //! \brief  Retrieves the identifier of the opengl buffer object.
            //! \return The identifier of the opengl buffer object..
            inline GLuint BufferObject::getId() const {
                return m_bufferId;
            }

            //! \brief  Gets a boolean that specifies whether or not the buffer contains dynamic data.
            //! \return <em>True</em> if the buffer data is suitable for dynamic updating otherwise <em>false</em>.
            inline bool BufferObject::isDynamic() const {
                return m_isDynamic;
            }

            //! \brief  Gets the length of data the buffer can contain.
            //! \return The length (in bytes) of data the buffer object may contain.
            inline size_t BufferObject::getDataLength() const {
                return m_dataLength;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_OPENGL_BUFFER_OBJECT_H
