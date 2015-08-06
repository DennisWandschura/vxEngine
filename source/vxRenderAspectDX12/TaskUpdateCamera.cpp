#include "TaskUpdateCamera.h"
#include <vxLib/Allocator/Allocator.h>
#include <vxLib/Graphics/Camera.h>

TaskUpdateCamera::TaskUpdateCamera(const __m128 &position, const __m128 &quaternionRotation, vx::Camera* camera)
	:m_position(position),
	m_quaternionRotation(quaternionRotation),
	m_camera(camera)
{
}

TaskUpdateCamera::~TaskUpdateCamera()
{

}

TaskReturnType TaskUpdateCamera::runImpl()
{
	m_camera->setPosition(m_position);
	m_camera->setRotation(m_quaternionRotation);

	/*auto projectionMatrix = m_renderContext.getProjectionMatrix();

	UniformCameraBufferBlock block;
	m_camera.getViewMatrix(&block.viewMatrix);
	block.pvMatrix = projectionMatrix * block.viewMatrix;
	block.inversePVMatrix = vx::MatrixInverse(block.pvMatrix);
	block.position = m_camera.getPosition();
	block.qrotation = m_camera.getRotation();

	m_cameraBuffer.subData(0, sizeof(UniformCameraBufferBlock), &block);*/

	return TaskReturnType::Success;
}

f32 TaskUpdateCamera::getTimeMs() const
{
	return 0.0f;
}