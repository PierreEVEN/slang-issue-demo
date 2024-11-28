#ifndef SLANG_COM_OBJECT_H
#define SLANG_COM_OBJECT_H

#include <atomic>
#include <iostream>

namespace Slang
{
//
// Use `SLANG_ASSUME(myBoolExpression);` to inform the compiler that the condition is true.
// Do not rely on side effects of the condition being performed.
//
#if defined(__cpp_assume)
#define SLANG_ASSUME(X) [[assume(X)]]
#elif SLANG_GCC
#define SLANG_ASSUME(X)              \
    do                               \
    {                                \
        if (!(X))                    \
            __builtin_unreachable(); \
    } while (0)
#elif SLANG_CLANG
#define SLANG_ASSUME(X) __builtin_assume(X)
#elif SLANG_VC
#define SLANG_ASSUME(X) __assume(X)
#else
[[noreturn]] inline void invokeUndefinedBehaviour()
{
}
#define SLANG_ASSUME(X)                 \
    do                                  \
    {                                   \
        if (!(X))                       \
            invokeUndefinedBehaviour(); \
    } while (0)
#endif

#ifdef _DEBUG
#define SLANG_ASSERT(VALUE)                            \
    do                                                 \
    {                                                  \
        if (!(VALUE))                                  \
        {                                              \
            std::cerr << "SLANG ERROR : " #VALUE "\n"; \
            exit(-1);                                  \
        }                                              \
    } while (0)
#else
#define SLANG_ASSERT(VALUE) SLANG_ASSUME(VALUE)
#endif
/// A base class for COM interfaces that require atomic ref counting
/// and are *NOT* derived from RefObject
class ComBaseObject
{
  public:
    /// If assigned the the ref count is *NOT* copied
    ComBaseObject& operator=(const ComBaseObject&)
    {
        return *this;
    }

    /// Copy Ctor, does not copy ref count
    ComBaseObject(const ComBaseObject&) : m_refCount(0)
    {
    }

    /// Default Ctor sets with no refs
    ComBaseObject() : m_refCount(0)
    {
    }

    /// Dtor needs to be virtual to avoid needing to
    /// Implement release for all derived types.
    virtual ~ComBaseObject()
    {
    }

  protected:
    inline uint32_t _releaseImpl();

    std::atomic<uint32_t> m_refCount;
};

// ------------------------------------------------------------------
inline uint32_t ComBaseObject::_releaseImpl()
{
    // Check there is a ref count to avoid underflow
    SLANG_ASSERT(m_refCount != 0);
    const uint32_t count = --m_refCount;
    if (count == 0)
    {
        delete this;
    }
    return count;
}

#define SLANG_COM_BASE_IUNKNOWN_QUERY_INTERFACE                                                                   \
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE \
    {                                                                                                             \
        void* intf = getInterface(uuid);                                                                          \
        if (intf)                                                                                                 \
        {                                                                                                         \
            ++m_refCount;                                                                                         \
            *outObject = intf;                                                                                    \
            return SLANG_OK;                                                                                      \
        }                                                                                                         \
        return SLANG_E_NO_INTERFACE;                                                                              \
    }
#define SLANG_COM_BASE_IUNKNOWN_ADD_REF                         \
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE \
    {                                                           \
        return ++m_refCount;                                    \
    }
#define SLANG_COM_BASE_IUNKNOWN_RELEASE                          \
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE \
    {                                                            \
        return _releaseImpl();                                   \
    }
#define SLANG_COM_BASE_IUNKNOWN_ALL         \
    SLANG_COM_BASE_IUNKNOWN_QUERY_INTERFACE \
    SLANG_COM_BASE_IUNKNOWN_ADD_REF         \
    SLANG_COM_BASE_IUNKNOWN_RELEASE

class RefObject
{
  private:
    uint32_t referenceCount;

  public:
    RefObject() : referenceCount(0)
    {
    }

    RefObject(const RefObject&) : referenceCount(0)
    {
    }

    RefObject& operator=(const RefObject&)
    {
        return *this;
    }

    virtual ~RefObject()
    {
    }

    uint32_t addReference()
    {
        return ++referenceCount;
    }

    uint32_t decreaseReference()
    {
        return --referenceCount;
    }

    uint32_t releaseReference()
    {
        SLANG_ASSERT(referenceCount != 0);
        if (--referenceCount == 0)
        {
            delete this;
            return 0;
        }
        return referenceCount;
    }

    bool isUniquelyReferenced()
    {
        SLANG_ASSERT(referenceCount != 0);
        return referenceCount == 1;
    }

    uint32_t debugGetReferenceCount()
    {
        return referenceCount;
    }
};

/// COM object that derives from RefObject
class ComObject : public RefObject
{
  protected:
    std::atomic<uint32_t> comRefCount;

  public:
    ComObject() : comRefCount(0)
    {
    }
    ComObject(const ComObject& rhs) : RefObject(rhs), comRefCount(0)
    {
    }

    ComObject& operator=(const ComObject&)
    {
        return *this;
    }

    virtual void comFree()
    {
    }

    uint32_t addRefImpl()
    {
        auto oldRefCount = comRefCount++;
        if (oldRefCount == 0)
            addReference();
        return oldRefCount + 1;
    }

    uint32_t releaseImpl()
    {
        auto oldRefCount = comRefCount--;
        if (oldRefCount == 1)
        {
            comFree();
            releaseReference();
        }
        return oldRefCount - 1;
    }
};

#define SLANG_COM_OBJECT_IUNKNOWN_QUERY_INTERFACE                                                                 \
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE \
    {                                                                                                             \
        void* intf = getInterface(uuid);                                                                          \
        if (intf)                                                                                                 \
        {                                                                                                         \
            addRef();                                                                                             \
            *outObject = intf;                                                                                    \
            return SLANG_OK;                                                                                      \
        }                                                                                                         \
        return SLANG_E_NO_INTERFACE;                                                                              \
    }
#define SLANG_COM_OBJECT_IUNKNOWN_ADD_REF                       \
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE \
    {                                                           \
        return addRefImpl();                                    \
    }
#define SLANG_COM_OBJECT_IUNKNOWN_RELEASE                        \
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE \
    {                                                            \
        return releaseImpl();                                    \
    }
#define SLANG_COM_OBJECT_IUNKNOWN_ALL         \
    SLANG_COM_OBJECT_IUNKNOWN_QUERY_INTERFACE \
    SLANG_COM_OBJECT_IUNKNOWN_ADD_REF         \
    SLANG_COM_OBJECT_IUNKNOWN_RELEASE

} // namespace Slang

#endif
