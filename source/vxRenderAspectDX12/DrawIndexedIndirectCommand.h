#pragma once

struct ID3D12CommandSignature;
struct ID3D12GraphicsCommandList;
struct D3D12_DRAW_INDEXED_ARGUMENTS;
class UploadManager;

namespace d3d
{
	class ResourceManager;
}

#include "d3d.h"

class DrawIndexedIndirectCommand
{
	d3d::Object<ID3D12CommandSignature> m_commandSignature;
	ID3D12Resource* m_commmandBuffer;
	u32 m_count;
	u32 m_countOffset;
	u32 m_maxCount;

	bool createSignature(ID3D12Device* device);

public:
	DrawIndexedIndirectCommand();
	~DrawIndexedIndirectCommand();

	void getRequiredMemory(u32 maxCount, u64* bufferHeapSize);

	bool create(const wchar_t* id, u32 maxCount, d3d::ResourceManager* resourceManager, ID3D12Device* device);
	void destroy();

	void uploadDrawCommand(u32 index, const D3D12_DRAW_INDEXED_ARGUMENTS &cmd, UploadManager* uploadManager);
	void setCount(u32 count, UploadManager* uploadManager);

	void incrementCount(UploadManager* uploadManager)
	{
		setCount(m_count + 1, uploadManager);
	}

	void draw(ID3D12GraphicsCommandList* cmdList);

	u32 getCount() const { return m_count; }
	u32 getMaxCount() const { return m_maxCount; }
};