#ifndef NGEN_DIRECTX12_COM_PTR
#define NGEN_DIRECTX12_COM_PTR

namespace ngen::directx12 {
    template <typename TType> class ComPtr {
    public:
        ComPtr();
        ~ComPtr();

        ComPtr(const ComPtr &other);
        ComPtr& operator=(const ComPtr &other);

        TType* operator->();
        const TType* operator->() const;

        bool isValid() const;

        void**();

        void release();

    private:
        TType *m_ptr;
    };

    template <typename TType>
    ComPtr<TType>::ComPtr() : m_ptr(nullptr) {}

    template <typename TType>
    ComPtr<TType>::~ComPtr() {
        if (m_ptr != nullptr) {
            m_ptr->Release();
        }
    }

    template <typename TType>
    ComPtr<TType>::ComPtr(const ComPtr<TType> &other) {
        m_ptr = other.m_ptr;
        if (m_ptr != nullptr) {
            m_ptr->AddRef();
        }
    }

    template <typename TType>
    ComPtr<TType>& ComPtr<TType>::operator=(const ComPtr<TType> &other) {
        m_ptr = other.m_ptr;
        if (m_ptr != nullptr) {
            m_ptr->AddRef();
        }
    }

    template <typename TType> ComPtr<TType>::void**() {
        return &m_ptr;
    };

    template <typename TType>
    void ComPtr<TType>::release() {
        if (m_ptr != nullptr) {
            m_ptr->Release();
            m_ptr = nullptr;
        }
    }

    template <typename TType>
    inline bool ComPtr<TType>::isValid() const {
        return m_ptr != nullptr;
    }

    template <typename TType>
    inline TType* ComPtr<TType>::operator->() {
        return m_ptr;
    }

    template <typename TType>
    inline const TType* ComPtr<TType>::operator->() const {
        return m_ptr;
    }
}

#endif //NGEN_DIRECTX12_COM_PTR
