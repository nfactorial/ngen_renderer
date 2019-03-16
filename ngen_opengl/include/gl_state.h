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

#ifndef NGEN_GL_STATE_H
#define NGEN_GL_STATE_H

////////////////////////////////////////////////////////////////////////////

#include "platform.h"
#include "cull_mode.h"

////////////////////////////////////////////////////////////////////////////

namespace ngen {
    namespace rendering {
        namespace ogl {
            //! \brief Manages the rendering state of the current OpenGL device.
            class State {
            public:
                State();
                ~State() = default;

                void reset();

                GLclampf getClearDepth() const;
                void setClearDepth(GLclampf value);

                int getClearStencil() const;
                void setClearStencil(GLint value);

                bool getDepthRead() const;
                void setDepthRead(bool enabled);

                void bindArrayBuffer(GLuint id);
                void bindElementArrayBuffer(GLuint id);

                void useProgram(GLuint id);

                void setCullMode(kCullMode cullMode);

            private:
                GLuint m_elementArrayBuffer;
                GLuint m_activeProgram;
                GLuint m_arrayBuffer;
                GLint m_clearStencil;
                GLclampf m_clearDepth;
                kCullMode m_cullMode;
                bool m_depthRead;
            };

            /**
             * Retrieves the current depth value enabled on the device.
             * @return Value the depth buffer will be cleared to during a clear operation.
             */
            inline GLclampf State::getClearDepth() const {
                return m_clearDepth;
            }

            /**
             * Retrieves the current depth value enabled on the device.
             * @return Value the depth buffer will be cleared to during a clear operation.
             */
            inline GLint State::getClearStencil() const {
                return m_clearStencil;
            }

            /**
             * Retrieves the current depth value enabled on the device.
             * @return Value the depth buffer will be cleared to during a clear operation.
             */
            inline bool State::getDepthRead() const {
                return m_depthRead;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_GL_STATE_H
