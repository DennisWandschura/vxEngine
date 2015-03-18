#pragma once

#include "Waypoint.h"
#include <vector>

class EditorRenderAspect;

namespace Editor
{
	class WaypointManager
	{
		std::vector<Waypoint> m_waypoints;

	public:
		void addWaypoint(const vx::float3 &p, EditorRenderAspect* r);
	};
}