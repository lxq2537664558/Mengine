#pragma once

#include "Kernel/Mixin.h"

//////////////////////////////////////////////////////////////////////////
namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    class LoadableInterface
        : public Mixin
    {
    public:
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<LoadableInterface> LoadableInterfacePtr;
    //////////////////////////////////////////////////////////////////////////
}