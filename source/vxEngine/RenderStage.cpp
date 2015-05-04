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
#include "RenderStage.h"
#include "Capability.h"

namespace Graphics
{
	void RenderStage::draw() const
	{
		for (auto &it : m_renderPasses)
		{
			for (auto &cap : it.capablitiesBegin)
			{
				cap->set();
			}

			it.pass.draw();

			for (auto &cap : it.capablitiesEnd)
			{
				cap->set();
			}
		}
	}

	void RenderStage::pushRenderPass(const char* id, const RenderPass &renderPass)
	{
		U32 index = m_renderPasses.size();

		Pass p;
		p.pass = renderPass;

		m_renderPasses.push_back(p);
		m_indices.insert(vx::make_sid(id), index);
	}

	RenderStage::Pass* RenderStage::findPass(const char* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_indices.find(sid);

		Pass* p = (it == m_indices.end()) ? nullptr : &m_renderPasses[*it];

		return p;
	}

	void RenderStage::attachCapabilityBegin(const char* id, Capability* cap)
	{
		auto pass = findPass(id);
		if (pass)
		{
			pass->capablitiesBegin.push_back(cap);
		}
	}

	void RenderStage::attachCapabilityEnd(const char* id, Capability* cap)
	{
		auto pass = findPass(id);
		if (pass)
		{
			pass->capablitiesEnd.push_back(cap);
		}
	}

	void RenderStage::setDrawCountAll(U32 count)
	{
		for (auto &it : m_renderPasses)
		{
			it.pass.setDrawCount(count);
		}
	}

	void RenderStage::setDrawCount(const char* id, U32 count)
	{
		auto pass = findPass(id);
		if (pass)
		{
			pass->pass.setDrawCount(count);
		}
	}
}