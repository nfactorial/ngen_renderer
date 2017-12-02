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

#if !defined(NGEN_HEADER_INCLUDED_NFACTORIAL)
#define NGEN_HEADER_INCLUDED_NFACTORIAL

#define NGEN_NOOP       ((void)0)

#if defined(_MSC_VER)
    #define NGEN_DEBUG_BREAK()  __debugbreak()
#else
    // TODO: Implement debug break for other platforms
    #define NGEN_DEBUG_BREAK()      NGEN_NOOP
#endif //defined(WINDOWS)


#if defined(_DEBUG)
    // TODO: Log assertion failure to output
    #define NGEN_ASSERT(expr)           \
    if (!expr) {                        \
        NGEN_DEBUG_BREAK();             \
    } else {}

    #define NGEN_VERIFY(expr)           \
    if (!expr) {                        \
        NGEN_DEBUG_BREAK();             \
    } else {}
#else
    #define NGEN_ASSERT(expr)           \
            NGEN_NOOP

    #define NGEN_VERIFY(expr)           \
            expr
#endif //defined(_DEBUG)

#endif //!defined(NGEN_HEADER_INCLUDED_NFACTORIAL)
