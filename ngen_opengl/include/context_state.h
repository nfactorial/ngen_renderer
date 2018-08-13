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

#ifndef NGEN_OPENGL_CONTEXT_STATE_H
#define NGEN_OPENGL_CONTEXT_STATE_H

////////////////////////////////////////////////////////////////////////////

#include "depth_compare.h"
#include "cull_mode.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen {
    namespace rendering {
        namespace ogl {
            //! \brief This object manages the current state of the open gl context.
            //!
            //! The ContextState object works directly with OpenGL values rather than nGen's own definitions.
            //! It is not intended to be used at the high level, and should be accessed by lower level systems
            //! that know about OpenGL itself.
            class ContextState final {
                struct EnableState {
                    bool isValid;
                    bool isEnabled;
                };

            public:
                ContextState();
                ~ContextState();

                void invalidate();

                void useProgram(GLuint programId);

                bool getDepthWrite() const;
                void setDepthWrite(bool enable);

                void setDepthTest(bool enable);
                void setDepthFunc(kDepthCompare comparison);

                bool getDepthTest() const;
                kDepthCompare getDepthFunc() const;

                bool getCullFace() const;
                void setCullFace(bool enable);

                kCullMode getCullMode() const;
                void setCullMode(kCullMode cullMode);

            private:
                GLuint  m_activeProgram;

                EnableState m_depthWrite;
                EnableState m_depthTest;
                EnableState m_cullFace;

                bool m_depthFuncValid;
                kDepthCompare m_depthFunc;

                bool m_cullModeValid;
                kCullMode m_cullMode;
            };

            //! \brief  Determines whether or not depth testing is currently enabled.
            //! \returns <em>True</em> if depth testing is enabled otherwise <em>false</em>.
            inline bool ContextState::getCullFace() const {
                return m_cullFace.isValid ? m_cullFace.isEnabled : false;
            }

            //! \brief  Determines whether or not depth testing is currently enabled.
            //! \returns <em>True</em> if depth testing is enabled otherwise <em>false</em>.
            inline kCullMode ContextState::getCullMode() const {
                return m_cullModeValid ? m_cullMode : kCullMode_Back;
            }

            //! \brief  Determines whether or not writing to the depth buffer is currently enabled.
            //! \returns <em>True</em> if writing to the depth buffer is enabled otherwise <em>false</em>.
            inline bool ContextState::getDepthWrite() const {
                return m_depthWrite.isValid ? m_depthWrite.isEnabled : false;
            }

            //! \brief  Determines whether or not depth testing is currently enabled.
            //! \returns <em>True</em> if depth testing is enabled otherwise <em>false</em>.
            inline bool ContextState::getDepthTest() const {
                return m_depthTest.isValid ? m_depthTest.isEnabled : false;
            }

            //! \brief  Retrieves the current depth comparison method applied to the rendering pipeline.
            //! \returns The depth comparison operation currently used by the rendering pipeline.
            inline kDepthCompare ContextState::getDepthFunc() const {
                return m_depthFuncValid ? m_depthFunc : kDepthCompare_LessEqual;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_OPENGL_CONTEXT_STATE_H
