#	include "Factory.h"
#	include "Factorable.h"

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	Factory::Factory()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	Factory::~Factory()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	Factorable * Factory::createObject()
	{
		Factorable * object = this->_createObject();
		object->setFactory( this );
		return object;
	}
}
