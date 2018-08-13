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

#ifndef NGEN_OPENGL_DEPTH_COMPARE
#define NGEN_OPENGL_DEPTH_COMPARE

////////////////////////////////////////////////////////////////////////////

#include "platform.h"


////////////////////////////////////////////////////////////////////////////

namespace ngen {
    namespace rendering {
        namespace ogl {
            enum kDepthCompare {
                kDepthCompare_Never = GL_NEVER,
                kDpethCompare_Less = GL_LESS,
                kDepthCompare_Equal = GL_EQUAL,
                kDepthCompare_LessEqual = GL_LEQUAL,
                kDepthCompare_Greater = GL_GREATER,
                kDepthCompare_NotEqual = GL_NOTEQUAL,
                kDepthCompare_GreaterEqual = GL_GEQUAL,
                kDepthCompare_Always = GL_ALWAYS
            };
        }
    }
}

////////////////////////////////////////////////////////////////////////////

#endif //NGEN_OPENGL_DEPTH_COMPARE
