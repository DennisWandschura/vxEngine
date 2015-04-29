#include "CapabilityDisableDepthTest.h"
#include <vxLib/gl/StateManager.h>

namespace Graphics
{
	CapabilityDisableDepthTest::CapabilityDisableDepthTest()
		:Capability()
	{
	}

	void CapabilityDisableDepthTest::set()
	{
		vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	}
}