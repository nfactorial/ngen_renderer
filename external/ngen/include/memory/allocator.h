//
// Copyright 2019 nfactorial
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

#if !defined(NGEN_ALLOCATOR_INCLUDED_NFACTORIAL)
#define NGEN_ALLOCATOR_INCLUDED_NFACTORIAL

struct IAllocator {
    virtual void* malloc(size_t dataLength) = 0;
    virtual void* malloc(size_t dataLength, const char *fileName, size_t line) = 0;

    virtual void* aligned_malloc(size_t dataLength, size_t alignment) = 0;
    virtual void* aligned_malloc(size_t dataLength, size_t alignment, const char *fileName, size_t line) = 0;
};


#endif //!defined(NGEN_ALLOCATOR_INCLUDED_NFACTORIAL)
