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

#include "context_state.h"
#include "../include/context_state.h"

namespace ngen {
    namespace rendering {
        namespace ogl {
            ContextState::ContextState()
            : m_activeProgram(0)
            , m_depthFuncValid(false)
            , m_cullModeValid(false)
            {
                m_cullFace.isValid = false;
                m_depthTest.isValid = false;
                m_depthWrite.isValid = false;
            }

            ContextState::~ContextState() {
            }

            //! \brief Informs the state that its knowledge of the context is no longer valid.
            void ContextState::invalidate() {
                m_activeProgram = 0;

                m_depthFuncValid = false;
                m_depthTest.isValid = false;
                m_depthWrite.isValid = false;

                m_cullModeValid = false;
                m_cullFace.isValid = false;
            }

            //! \brief Enables the specified program on the OpenGL context we represent.
            //! \param programId [in] - Identifier of the OpenGL shader program to be enabled.
            void ContextState::useProgram(GLuint programId) {
                if (programId != m_activeProgram) {
                    glUseProgram(programId);
                    m_activeProgram = programId;
                }
            }

            //! \brief  Enables or disables writing to the depth buffer during rendering.
            //! \param  enable - True if depth writing is to be enabled otherwise false.
            void ContextState::setDepthWrite(bool enable) {
                if (!m_depthWrite.isValid || m_depthWrite.isEnabled != enable) {
                    m_depthWrite.isValid = true;
                    m_depthWrite.isEnabled = enable;

                    // It is possible GL_TRUE and GL_FALSE are not 1/0, so we don't use the boolean directly.
                    if (enable) {
                        glDepthMask(GL_TRUE);
                    } else {
                        glDepthMask(GL_FALSE);
                    }
                }
            }

            //! \brief  Enables or disables the depth test functionality of the rendering context.
            //! \param  enable - True if depth testing is to be enabled otherwise false.
            void ContextState::setDepthTest(bool enable) {
                if (!m_depthTest.isValid || m_depthTest.isEnabled != enable) {
                    m_depthTest.isValid = true;
                    m_depthTest.isEnabled = enable;

                    if (enable) {
                        glEnable(GL_DEPTH_TEST);
                    } else {
                        glDisable(GL_DEPTH_TEST);
                    }
                }
            }

            //! \brief  Sets the depth comparison operation to be performed by the rendering context.
            //! \param  comparison - The depth comparison operation to be applied within the rendering context.
            void ContextState::setDepthFunc(kDepthCompare comparison) {
                if (!m_depthFuncValid || m_depthFunc != comparison) {
                    m_depthFuncValid = true;
                    m_depthFunc = comparison;

                    glDepthFunc(comparison);
                }
            }

            //! \brief  Enables or disables the face culling functionality of the rendering context.
            //! \param  enable - True if depth culling is to be enabled otherwise false.
            void ContextState::setCullFace(bool enable) {
                if (!m_cullFace.isValid || m_cullFace.isEnabled != enable) {
                    m_cullFace.isValid = true;
                    m_cullFace.isEnabled = enable;

                    if (enable) {
                        glEnable(GL_CULL_FACE);
                    } else {
                        glDisable(GL_CULL_FACE);
                    }
                }
            }

            //! \brief  Sets the culling operation to be performed on rendered primitives.
            //! \param cullMode - The culling operation to be performed.
            void ContextState::setCullMode(kCullMode cullMode) {
                if (!m_cullModeValid || m_cullMode != cullMode) {
                    m_cullModeValid = true;
                    m_cullMode = cullMode;

                    glCullFace(cullMode);
                }
            }
        }
    }
}
