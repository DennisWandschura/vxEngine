#include "TaskUpdateCamera.h"
#include <vxLib/Allocator/Allocator.h>
#include <vxLib/Graphics/Camera.h>

TaskUpdateCamera::TaskUpdateCamera(u32 tid, const __m128 &position, const __m128 &quaternionRotation, vx::Camera* camera)
	:Task(tid),
	m_position(position),
	m_quaternionRotation(quaternionRotation),
	m_camera(camera)
{
}

TaskUpdateCamera::TaskUpdateCamera(TaskUpdateCamera &&rhs)
	:Task(std::move(rhs)),
	m_position(rhs.m_position),
	m_quaternionRotation(rhs.m_quaternionRotation),
	m_camera(rhs.m_camera)
{

}

TaskUpdateCamera::~TaskUpdateCamera()
{

}

TaskReturnType TaskUpdateCamera::run()
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

Task* TaskUpdateCamera::move(vx::Allocator* allocator)
{
	auto ptr = (TaskUpdateCamera*)allocator->allocate(sizeof(TaskUpdateCamera), __alignof(TaskUpdateCamera));

	if (ptr != nullptr)
	{
		new (ptr) TaskUpdateCamera(std::move(*this));
	}

	return ptr;
}