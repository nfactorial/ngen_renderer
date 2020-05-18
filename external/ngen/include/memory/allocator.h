
#if !defined(NGEN_ALLOCATOR_INCLUDED_NFACTORIAL)
#define NGEN_ALLOCATOR_INCLUDED_NFACTORIAL

struct IAllocator {
    virtual void* malloc(size_t dataLength) = 0;
    virtual void* malloc(size_t dataLength, const char *fileName, size_t line) = 0;

    virtual void* aligned_malloc(size_t dataLength, size_t alignment) = 0;
    virtual void* aligned_malloc(size_t dataLength, size_t alignment, const char *fileName, size_t line) = 0;
};


#endif //!defined(NGEN_ALLOCATOR_INCLUDED_NFACTORIAL)
