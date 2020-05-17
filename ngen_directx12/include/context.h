#ifndef NGEN_DIRECTX12_CONTEXT
#define NGEN_DIRECTX12_CONTEXT

#include "com_ptr.h"

namespace ngen::directx12 {
    class Context {
    public:
        Context() = default;
        ~Context() = default;

        Context(const Context &other) = delete;
        const Context& operator=(const Context &other) = delete;

        bool initialize();

    private:
        ComPtr<IDXGIFactory4> m_factory;
        ComPtr<IDXGIAdapter1> m_adapter;
    };
}

#endif //NGEN_DIRECTX12_CONTEXT
