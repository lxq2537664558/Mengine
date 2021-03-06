#pragma once

#include "Interface/DataflowInterface.h"
#include "Interface/ArchivatorInterface.h"

#include "Kernel/Factorable.h"
#include "Kernel/Factory.h"

#include "pybind/pybind.hpp"

namespace Mengine
{
    class DataflowPYZ
        : public DataflowInterface
        , public Factorable
    {
    public:
        DataflowPYZ();
        ~DataflowPYZ() override;

    public:
        void setKernel( pybind::kernel_interface * _kernel );
        pybind::kernel_interface * getKernel() const;

        void setArchivator( const ArchivatorInterfacePtr & _archivator );
        const ArchivatorInterfacePtr & getArchivator() const;

    public:
        bool initialize() override;
        void finalize() override;

    public:
        DataInterfacePtr create( const Char * _doc ) override;

    public:
        bool load( const DataInterfacePtr & _data, const InputStreamInterfacePtr & _stream, const Char * _doc ) override;

    protected:
        pybind::kernel_interface * m_kernel;

        ArchivatorInterfacePtr m_archivator;

        FactoryPtr m_factoryScriptCodeData;
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<DataflowPYZ> DataflowPYZPtr;
    //////////////////////////////////////////////////////////////////////////
}