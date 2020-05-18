
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
