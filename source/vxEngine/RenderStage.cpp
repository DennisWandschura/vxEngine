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