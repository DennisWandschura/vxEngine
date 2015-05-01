#pragma once

class GpuProfiler;

#include "RenderPass.h"
#include <vector>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>

namespace Graphics
{
	class Capability;

	class RenderStage
	{
		struct Pass
		{
			std::vector<Capability*> capablitiesBegin;
			RenderPass pass;
			std::vector<Capability*> capablitiesEnd;
		};

		std::vector<Pass> m_renderPasses;
		vx::sorted_vector<vx::StringID64, U32> m_indices;

		Pass* findPass(const char* id);

	public:
		void draw() const;

		void pushRenderPass(const char* id, const RenderPass &renderPass);
		void attachCapabilityBegin(const char* id, Capability* cap);
		void attachCapabilityEnd(const char* id, Capability* cap);

		void setDrawCountAll(U32 count);
		void setDrawCount(const char* id, U32 count);
	};
}