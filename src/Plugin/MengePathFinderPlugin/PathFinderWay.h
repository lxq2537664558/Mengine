#	pragma once

#	include "Interface/RenderSystemInterface.h"

#	include "Math/vec2.h"

#	include "fastpathfinder/pathfinder.h"
#	include "fastpathfinder/map.h"

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	typedef stdex::vector<mt::vec2f> TVectorWayPoint;
	//////////////////////////////////////////////////////////////////////////
	class PathFinderWay
		: public FactorablePtr
	{
	public:
		PathFinderWay();
		~PathFinderWay();
		
	public:
		void setServiceProvider( ServiceProviderInterface * _serviceProvider );

	public:
		void initialize( const mt::vec2f & _from, const mt::vec2f & _to, float _gridSize, const fastpathfinder::point_array & _wp );
		
	public:
		void render( const RenderViewportInterface * _viewport, const RenderCameraInterface * _camera );

	public:
		inline size_t getWayPointCount() const;
		inline const mt::vec2f & getWayPoint( size_t _index ) const;

	protected:
		ServiceProviderInterface * m_serviceProvider;

		mt::vec2f m_from;
		mt::vec2f m_to;
		float m_gridSize;

		TVectorWayPoint m_way;
	};
	//////////////////////////////////////////////////////////////////////////
	typedef stdex::intrusive_ptr<PathFinderWay> PathFinderWayPtr;
	//////////////////////////////////////////////////////////////////////////
	inline size_t PathFinderWay::getWayPointCount() const
	{
		size_t size = m_way.size();

		return size;
	}
	//////////////////////////////////////////////////////////////////////////
	inline const mt::vec2f & PathFinderWay::getWayPoint( size_t _index ) const
	{
		const mt::vec2f & p = m_way[_index];

		return p;
	}

}

