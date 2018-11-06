#pragma once

#include "Interface/RenderProgramVariableInterface.h"

#include "Kernel/Factorable.h"

#include "Config/Vector.h"

#include <d3d9.h>

namespace Mengine
{
    class DX9RenderProgramVariable
        : public RenderProgramVariableInterface
        , public Factorable
    {
    public:
        DX9RenderProgramVariable();
        ~DX9RenderProgramVariable() override;

    public:
        bool initialize( uint32_t _vertexCount, uint32_t _pixelCount );
        void finalize();

    public:
        void setVertexVariableFloats( uint32_t _index, float * _values, uint32_t _count ) override;
        void setVertexVariableIntegers( uint32_t _index, int32_t * _values, uint32_t _count ) override;
        void setVertexVariableBooleans( uint32_t _index, int32_t * _values, uint32_t _count ) override;

    public:
        void setPixelVariableFloats( uint32_t _index, float * _values, uint32_t _count ) override;
        void setPixelVariableIntegers( uint32_t _index, int32_t * _values, uint32_t _count ) override;
        void setPixelVariableBooleans( uint32_t _index, int32_t * _values, uint32_t _count ) override;

    public:
        bool apply( IDirect3DDevice9 * _pD3DDevice, const RenderProgramInterfacePtr & _program );

    protected:
        typedef Vector<float> VectorDataFloats;
        typedef Vector<int32_t> VectorDataIntegers;
        typedef Vector<int32_t> VectorDataBooleans;

        VectorDataFloats m_dataFloats;
        VectorDataIntegers m_dataIntegers;
        VectorDataBooleans m_dataBooleans;

        VectorDataFloats m_pixelFloats;
        VectorDataIntegers m_pixelIntegers;
        VectorDataBooleans m_pixelBooleans;

        struct Variable
        {
            uint32_t type;
            uint32_t offset;
            uint32_t count;
        };

        typedef Vector<Variable> VectorVariables;
        VectorVariables m_vertexVariables;
        VectorVariables m_pixelVariables;
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<DX9RenderProgramVariable> DX9RenderProgramVariablePtr;
}