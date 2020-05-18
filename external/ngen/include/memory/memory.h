
#if !defined(NGEN_MEMORY_HEADER_INCLUDED_NFACTORIAL)
#define NGEN_MEMORY_HEADER_INCLUDED_NFACTORIAL

#include "ngen.h"
#include "allocator.h"

//! \brief Attempts to allocate a block of memory of a specified length (in bytes) using the supplied allocator.
//! \param dataLength - The length (in bytes) of the memory block to be allocated.
//! \param allocator - The IAllocator object that is to supply the requested memory block.
//! \returns Pointer to the allocated memory block or nullptr if the memory could not be allocated.
inline void* operator new(size_t dataLength, IAllocator *allocator) {
    return allocator ? allocator->malloc(dataLength) : nullptr;
}


//! \brief Attempts to allocate a block of memory of a specified length (in bytes) using the supplied allocator.
//! \param dataLength - The length (in bytes) of the memory block to be allocated.
//! \param allocator - The IAllocator object that is to supply the requested memory block.
//! \param fileName - The path to the source file that attempted to allocate the memory block.
//! \param line - Line number within the source file that attempted to allocate the memory block.
//! \returns Pointer to the allocated memory block or nullptr if the memory could not be allocated.
inline void* operator new(size_t dataLength, IAllocator *allocator, const char *fileName, size_t line) {
    void *result = allocator ? allocator->malloc(dataLength) : nullptr;
    if (!result) {
        // Log allocation failure to output
    }

    return result;
}

//! \brief Attempts to allocate a block of memory of a specified length (in bytes) using the supplied allocator, the memory block will be aligned to the specified boundary.
//! \param dataLength - The length (in bytes) of the memory block to be allocated.
//! \param allocator - The IAllocator object that is to supply the requested memory block.
//! \param alignment - The byte alignment the allocated memory block should be matched with.
//! \returns Pointer to the allocated memory block or nullptr if the memory could not be allocated.
inline void* operator new(size_t dataLength, IAllocator *allocator, size_t alignment) {
    return allocator ? allocator->aligned_malloc(dataLength, alignment) : nullptr;
}

//! \brief Attempts to allocate a block of memory of a specified length (in bytes) using the supplied allocator.
//! \param dataLength - The length (in bytes) of the memory block to be allocated.
//! \param allocator - The IAllocator object that is to supply the requested memory block.
//! \param alignment - The byte alignment the allocated memory block should be matched with.
//! \param fileName - The path to the source file that attempted to allocate the memory block.
//! \param line - Line number within the source file that attempted to allocate the memory block.
//! \returns Pointer to the allocated memory block or nullptr if the memory could not be allocated.
inline void* operator new(size_t dataLength, IAllocator *allocator, size_t alignment, const char *fileName, size_t line) {
    void *result = allocator ? allocator->aligned_malloc(dataLength, alignment) : nullptr;
    if (!result) {
        // Log allocation failure to output
    }

    return result;
}

#if defined(_DEBUG)
    #define NGEN_NEW(allocator) new(allocator, __FILE__, __LINE__)
    #define NGEN_ALIGNED_NEW(allocator, alignment) new(allocator, alignment, __FILE__, __LINE__)
#else
    #define NGEN_NEW(allocator) new(allocator)
    #define NGEN_ALIGNED_NEW(allocator, alignment) new(allocator, alignment)
#endif //defined(_DEBUG)

#endif //!defined(NGEN_MEMORY_HEADER_INCLUDED_NFACTORIAL)
