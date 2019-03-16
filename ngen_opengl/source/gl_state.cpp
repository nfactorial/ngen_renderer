//
// Copyright 2018 nfactorial
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

#include "gl_state.h"

const int INVALID_BUFFER_ID = 0;
const int INVALID_PROGRAM = 0;

const bool DEFAULT_DEPTH_READ = true;
const GLint DEFAULT_CLEAR_STENCIL = 0;
const GLclampf DEFAULT_CLEAR_DEPTH = 1.0f;

namespace ngen {
    namespace rendering {
        namespace ogl {
            const kCullMode DEFAULT_CULL_MODE = kCullMode_Back;

            State::State()
            : m_elementArrayBuffer(INVALID_BUFFER_ID)
            , m_activeProgram(INVALID_PROGRAM)
            , m_arrayBuffer(INVALID_BUFFER_ID)
            , m_clearStencil(DEFAULT_CLEAR_STENCIL)
            , m_clearDepth(DEFAULT_CLEAR_DEPTH)
            , m_cullMode(DEFAULT_CULL_MODE)
            , m_depthRead(DEFAULT_DEPTH_READ) {
            }

            void State::reset() {
                // TODO: Set everything to their default values
            }

            //! \brief Sets the value the depth buffer will be cleared to.
            //! \param value [in] The value the depth buffer will be cleared to [0...1]
            void State::setClearDepth(GLclampf value) {
                if (value != m_clearDepth) {
                    m_clearDepth  = value;
                    glClearDepthf(value);
                }
            }

            //! \brief Sets the value the stencil buffer will be cleared to.
            //! \param value [in] The value the stencil buffer will be cleared to.
            void State::setClearStencil(GLint value) {
                if (value != m_clearStencil) {
                    m_clearStencil = value;
                    glClearStencil(value);
                }
            }

            //! \brief Enables or disables depth reading during rendering.
            //! \param enabled [in] - True to enable depth reading otherwise false to disable.
            void State::setDepthRead(bool enabled) {
                if (enabled != m_depthRead) {
                    m_depthRead = enabled;

                    if (enabled) {
                        glEnable(GL_DEPTH_TEST);
                    } else {
                        glDisable(GL_DEPTH_TEST);
                    }
                }
            }

            void State::setCullMode(kCullMode cullMode) {
                if (cullMode != m_cullMode) {
                    m_cullMode = cullMode;
                    glCullFace(cullMode);
                }
            }

            //! \brief Binds a buffer of vertex data to the rendering contexts array buffer input.
            //! \param id [in] - Identifier of the buffer to be bound.
            void State::bindArrayBuffer(GLuint id) {
                if (id != m_arrayBuffer) {
                    m_arrayBuffer = id;
                    glBindBuffer(GL_ARRAY_BUFFER, id);
                }
            }

            //! \brief Binds a buffer of vertex data to the rendering contexts element array buffer input.
            //! \param id [in] - Identifier of the buffer to be bound.
            void State::bindElementArrayBuffer(GLuint id) {
                if (id != m_elementArrayBuffer) {
                    m_elementArrayBuffer = id;
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
                }
            }

            //! \brief Sets the program to be used when rendering primitives.
            //! \param id [in] - Identifier of the program to be used during rendering.
            void State::useProgram(GLuint id) {
                if (id != m_activeProgram) {
                    glUseProgram(id);
                    m_activeProgram = id;
                }
            }
        }
    }
}
