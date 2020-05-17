#include "context.h"

namespace ngen::directx12 {
    bool Context::initialize() {
        const auto hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_factory));
        if (FAILED(hr)) {
            return false;
        }

        return false;
    }
}
