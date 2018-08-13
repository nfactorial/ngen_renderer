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

#ifndef NGEN_OPENGL_CULL_MODE
#define NGEN_OPENGL_CULL_MODE

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen {
    namespace rendering {
        namespace ogl {
            enum kCullMode {
                kCullMode_Front = GL_FRONT,
                kCullMode_Back = GL_BACK,
                kCullMode_Both = GL_FRONT_AND_BACK
            };
        }
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_OPENGL_CULL_MODE
