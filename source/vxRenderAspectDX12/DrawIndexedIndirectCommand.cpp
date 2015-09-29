#include "DrawIndexedIndirectCommand.h"
#include <d3d12.h>
#include "ResourceManager.h"
#include "UploadManager.h"

DrawIndexedIndirectCommand::DrawIndexedIndirectCommand()
	:m_commandSignature(),
	m_commmandBuffer(nullptr),
	m_count(0),
	m_countOffset(0),
	m_maxCount(0)
{

}

DrawIndexedIndirectCommand::DrawIndexedIndirectCommand(DrawIndexedIndirectCommand &&rhs)
	:m_commandSignature(std::move(rhs.m_commandSignature)),
	m_commmandBuffer(rhs.m_commmandBuffer),
	m_count(rhs.m_count),
	m_countOffset(rhs.m_countOffset),
	m_maxCount(rhs.m_maxCount)
{
	rhs.m_count = 0;
	rhs.m_maxCount = 0;
}

DrawIndexedIndirectCommand::~DrawIndexedIndirectCommand()
{

}

void DrawIndexedIndirectCommand::getRequiredMemory(u32 maxCount, u64* bufferHeapSize, u32* bufferCount)
{
	auto cmdSize = d3d::getAlignedSize(sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * maxCount, 256llu);

	auto bufferSize = d3d::getAlignedSize(cmdSize + 256llu, 64llu KBYTE);

	*bufferHeapSize += bufferSize;
	*bufferCount += 1;
}

bool DrawIndexedIndirectCommand::createSignature(ID3D12Device* device)
{
	D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1] = {};
	argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	D3D12_COMMAND_SIGNATURE_DESC cmdSigDesc;
	cmdSigDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
	cmdSigDesc.NodeMask = 0;
	cmdSigDesc.NumArgumentDescs = 1;
	cmdSigDesc.pArgumentDescs = argumentDescs;
	auto hresult = device->CreateCommandSignature(&cmdSigDesc, nullptr, IID_PPV_ARGS(m_commandSignature.getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool DrawIndexedIndirectCommand::create(const wchar_t* id, u32 maxCount, d3d::ResourceManager* resourceManager, ID3D12Device* device)
{
	if (!createSignature(device))
		return false;

	auto cmdSize = d3d::getAlignedSize(sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * maxCount, 256llu);
	auto bufferSize = d3d::getAlignedSize(cmdSize + 256llu, 64llu KBYTE);

	m_commmandBuffer = resourceManager->createBuffer(id, bufferSize, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT)->get();
	if (m_commmandBuffer == nullptr)
		return false;

	m_countOffset = cmdSize;
	m_maxCount = maxCount;

	return true;
}

void DrawIndexedIndirectCommand::destroy()
{
	m_commmandBuffer = nullptr;
	m_count = 0;
	m_commandSignature.destroy();
}

void DrawIndexedIndirectCommand::uploadDrawCommand(u32 index, const D3D12_DRAW_INDEXED_ARGUMENTS &cmd, UploadManager* uploadManager)
{
	VX_ASSERT(cmd.StartInstanceLocation < m_maxCount);

	auto cmdOffset = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * index;

	uploadManager->pushUploadBuffer((u8*)&cmd, m_commmandBuffer, cmdOffset, sizeof(D3D12_DRAW_INDEXED_ARGUMENTS), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
}

void DrawIndexedIndirectCommand::setCount(u32 count, UploadManager* uploadManager)
{
	VX_ASSERT(count <= m_maxCount);

	auto cmdSize = d3d::getAlignedSize(sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * m_maxCount, 256llu);
	uploadManager->pushUploadBuffer((u8*)&count, m_commmandBuffer, cmdSize, sizeof(u32), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	m_count = count;
}

void DrawIndexedIndirectCommand::draw(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->ExecuteIndirect(m_commandSignature.get(), m_count, m_commmandBuffer, 0, m_commmandBuffer, m_countOffset);
}