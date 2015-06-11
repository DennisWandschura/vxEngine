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
#include "RenderAspect.h"

ActionUpdateGpuTransform::ActionUpdateGpuTransform()
	:m_playerEntity(nullptr),
	m_pRenderAspect(nullptr)
{

}

ActionUpdateGpuTransform::ActionUpdateGpuTransform(EntityActor* playerEntity, RenderAspect* pRenderAspect)
	:m_playerEntity(playerEntity),
	m_pRenderAspect(pRenderAspect)
{

}

ActionUpdateGpuTransform::~ActionUpdateGpuTransform()
{

}

void ActionUpdateGpuTransform::run()
{
	__m128 quaternionRotation = { m_playerEntity->orientation.y, m_playerEntity->orientation.x, 0, 0 };
	quaternionRotation = vx::quaternionRotationRollPitchYawFromVector(quaternionRotation);

	RenderUpdateCameraData data;
	data.position = vx::loadFloat3(m_playerEntity->position);
	data.quaternionRotation = quaternionRotation;

	m_pRenderAspect->queueUpdateCamera(data);
}