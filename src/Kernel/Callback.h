#pragma once

#include "Kernel/Factorable.h"

namespace Mengine
{
    template<class T>
    class Callback
        : public Factorable
        , public T
    {
    public:
        Callback()
        {
        }

        ~Callback() override
        {
        }
    };
}