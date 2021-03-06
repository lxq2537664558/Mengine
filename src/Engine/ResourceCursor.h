#pragma once

#include "Kernel/Resource.h"

#include "Interface/MemoryInterface.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    class ResourceCursor
        : public Resource
    {
        DECLARE_VISITABLE( Resource );

    public:
        virtual const FilePath & getFilePath() const = 0;
        virtual const MemoryInterfacePtr & getBuffer() const = 0;
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<ResourceCursor> ResourceCursorPtr;
    //////////////////////////////////////////////////////////////////////////
}