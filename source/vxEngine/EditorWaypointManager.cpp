#include "EditorWaypointManager.h"
#include "EditorRenderAspect.h"

namespace Editor
{
	void WaypointManager::addWaypoint(const vx::float3 &p, EditorRenderAspect* r)
	{
		Waypoint newWaypoint;
		newWaypoint.position = p;

		auto offset = m_waypoints.size();

		m_waypoints.push_back(newWaypoint);

		r->editor_updateWaypoint(offset, 1, m_waypoints.data());
	}
}