#pragma once

#include "Interface/ServantInterface.h"

#include "Kernel/ConstString.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    class RenderVertexShaderInterface
        : public ServantInterface
    {
    public:
        virtual const ConstString & getName() const = 0;
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<RenderVertexShaderInterface> RenderVertexShaderInterfacePtr;
    //////////////////////////////////////////////////////////////////////////
}