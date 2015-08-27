/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ActionUpdateGpuTransform.h"
#include "Entity.h"
#include <vxEngineLib/RenderAspectInterface.h>

ActionUpdateGpuTransform::ActionUpdateGpuTransform(EntityHuman* playerEntity, RenderAspectInterface* pRenderAspect)
	:m_playerEntity(playerEntity),
	m_pRenderAspect(pRenderAspect)
{

}

ActionUpdateGpuTransform::~ActionUpdateGpuTransform()
{

}

void ActionUpdateGpuTransform::run()
{
	//__m128 quaternionRotation = { m_playerEntity->orientation.y, m_playerEntity->orientation.x, 0, 0 };
	//quaternionRotation = vx::quaternionRotationRollPitchYawFromVector(quaternionRotation);
	__m128 quaternionRotation = vx::loadFloat4(m_playerEntity->m_qRotation);
	auto position = vx::loadFloat3(m_playerEntity->m_position);

	RenderUpdateCameraData data;
	data.position = { position.m128_f32[0], position.m128_f32[1], position.m128_f32[2], position.m128_f32[3] };
	data.quaternionRotation = { quaternionRotation.m128_f32[0], quaternionRotation.m128_f32[1], quaternionRotation.m128_f32[2], quaternionRotation.m128_f32[3] };;

	m_pRenderAspect->queueUpdateCamera(data);
}