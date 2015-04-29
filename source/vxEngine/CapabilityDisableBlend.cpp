#include "CapabilityDisableBlend.h"
#include <vxLib/gl/StateManager.h>

namespace Graphics
{
	void CapabilityDisableBlend::set()
	{
		vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
	}
}