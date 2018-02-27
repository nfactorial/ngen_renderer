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

#ifndef NGEN_OPENGL_TEXTURE_H
#define NGEN_OPENGL_TEXTURE_H

////////////////////////////////////////////////////////////////////////////

#include "platform.h"

////////////////////////////////////////////////////////////////////////////

namespace ngen {
    namespace rendering {
        namespace ogl {
            class Texture {
            public:
                Texture();
                ~Texture();

                void dispose();

                bool create(int width, int height);

                GLuint getId() const;

            private:
                GLuint      m_textureId;
            };

            //! \brief  Retrieves the identifier of the opengl texture.
            //! \return The identifier of the opengl texture.
            inline GLuint Texture::getId() const {
                return m_textureId;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_OPENGL_TEXTURE_H
