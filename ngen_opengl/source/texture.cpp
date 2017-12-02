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

#include "texture.h"

namespace ngen {
    namespace rendering {
        namespace ogl {
            Texture::Texture()
            : m_textureId(GL_INVALID_VALUE) {
                //
            }

            Texture::~Texture() {
                if (GL_INVALID_VALUE != m_textureId) {
                    glDeleteTextures(1, &m_textureId);
                }
            }

            //! \brief Deletes any resources currently in use by this object.
            void Texture::dispose() {
                if (GL_INVALID_VALUE != m_textureId) {
                    glDeleteTextures(1, &m_textureId);
                    m_textureId  = GL_INVALID_VALUE;
                }
            }

            //! \brief Attempts to allocate a texture for use by the running application.
            bool Texture::create() {
                if (GL_INVALID_VALUE != m_textureId) {
                    // TODO: Log
                    //printf("Texture::create - Texture already created.");
                    return false;
                }

                //glBindTexture(GL_TEXTURE_2D, m_textureId);
                //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
                //glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

                glGenTextures(1, &m_textureId);
                return GL_INVALID_VALUE != m_textureId;
            }
        }
    }
}
