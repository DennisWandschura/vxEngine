#include "CapabilityDisablePolygonOffsetFill.h"
#include <vxLib/gl/StateManager.h>

namespace Graphics
{
	void CapabilityDisablePolygonOffsetFill::set()
	{
		vx::gl::StateManager::disable(vx::gl::Capabilities::Polygon_Offset_Fill);
	}
}