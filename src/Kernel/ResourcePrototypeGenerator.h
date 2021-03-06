#pragma once

#include "ScriptablePrototypeGenerator.h"

#include "Kernel/Resource.h"

#include "Kernel/Logger.h"

namespace Mengine
{
    template<class Type, uint32_t Count>
    class ResourcePrototypeGenerator
        : public ScriptablePrototypeGenerator<Type, Count>
    {
    protected:
        FactorablePointer generate( const Char * _doc ) override
        {
            const FactoryPtr & factory = this->getFactory();

            ResourcePtr resource = factory->createObject( _doc );

            if( resource == nullptr )
            {
                LOGGER_ERROR( "can't generate '%s' '%s' doc '%s'"
                    , this->getCategory().c_str()
                    , this->getPrototype().c_str()
					, _doc
                );

                return nullptr;
            }

            const ConstString & prototype = this->getPrototype();
            resource->setType( prototype );

            this->setupScriptable( resource );

            return resource;
        }
    };
}