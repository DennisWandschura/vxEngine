#pragma once

#include <vxLib/types.h>
#include <d3d12.h>

namespace d3d
{
	struct ResourceView
	{
		enum class Type : u32
		{
			VertexBufferView,
			IndexBufferView
		};

		Type type;
		union
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;
			D3D12_INDEX_BUFFER_VIEW ibv;
		};
	};
}