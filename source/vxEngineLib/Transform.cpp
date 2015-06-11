#include <vxEngineLib/Transform.h>

namespace vx
{
	void TransformOld::convertTo(Transform* transform)
	{
		auto angles = vx::loadFloat3(m_rotation);
		auto q = vx::quaternionRotationRollPitchYawFromVector(angles);

		transform->m_translation = m_translation;
		_mm_storeu_ps(&transform->m_qRotation.x, q);
		transform->m_scaling = m_scaling;
	}
}