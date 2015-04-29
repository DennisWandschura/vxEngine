#include "CapabilityEnableDepthTest.h"
#include <vxLib/gl/StateManager.h>

namespace Graphics
{
	CapabilityEnableDepthTest::CapabilityEnableDepthTest()
		:Capability()
	{
	}

	void CapabilityEnableDepthTest::set()
	{
		vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
	}
}