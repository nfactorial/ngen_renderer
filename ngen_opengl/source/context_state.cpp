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

namespace ngen {
    namespace rendering {
        struct StateInfo {
            bool isValid;
            GLuint value;
        };

        namespace ogl {
            ContextState::ContextState()
            : m_activeProgram(0)
            {
                //
            }

            ContextState::~ContextState() {
            }

            //! \brief Informs the state that its knowledge of the context is no longer valid.
            void ContextState::invalidate() {
                m_activeProgram = 0;
            }

            //! \brief Enables the specified program on the OpenGL context we represent.
            //! \param programId [in] - Identifier of the OpenGL shader program to be enabled.
            void ContextState::useProgram(GLuint programId) {
                if (programId != m_activeProgram) {
                    glUseProgram(programId);
                    m_activeProgram = programId;
                }
            }
        }
    }
}
