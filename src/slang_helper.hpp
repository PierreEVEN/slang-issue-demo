#pragma once
#include "slang_com_object.hpp"

#include <slang-com-helper.h>
#include <slang.h>

/** Base class for simple blobs.
 */
class BlobBase : public ISlangBlob, public Slang::ComBaseObject
{
  public:
    // ISlangUnknown
    SLANG_COM_BASE_IUNKNOWN_ALL

  protected:
    ISlangUnknown* getInterface(const Slang::Guid& guid)
    {
        if (guid == ISlangUnknown::getTypeGuid() || guid == ISlangBlob::getTypeGuid())
        {
            return static_cast<ISlangBlob*>(this);
        }
        return nullptr;
    }

    static void* getObject(const Slang::Guid& guid)
    {
        SLANG_UNUSED(guid);
        return nullptr;
    }
};

class UnownedRawBlob : public BlobBase
{
  public:
    // ISlangBlob
    SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE
    {
        return m_data;
    }

    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE
    {
        return m_dataSizeInBytes;
    }

    static Slang::ComPtr<UnownedRawBlob> create(void const* inData, size_t size)
    {
        return Slang::ComPtr<UnownedRawBlob>(new UnownedRawBlob(inData, size));
    }

  protected:
    // Ctor
    UnownedRawBlob(const void* data, size_t size) : m_data(data), m_dataSizeInBytes(size)
    {
    }

    UnownedRawBlob() = default;

    const void* m_data;
    size_t      m_dataSizeInBytes;
};
