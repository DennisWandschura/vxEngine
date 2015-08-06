#pragma once

namespace vx
{
	class Camera;
}

#include <vxEngineLib/Task.h>
#include <vxLib/math/Vector.h>

class TaskUpdateCamera : public Task
{
	__m128 m_position;
	__m128 m_quaternionRotation;
	vx::Camera* m_camera;

	TaskReturnType runImpl() override;

public:
	TaskUpdateCamera(const __m128 &position, const __m128 &quaternionRotation, vx::Camera* camera);
	~TaskUpdateCamera();

	f32 getTimeMs() const override;
};