#pragma once

struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;

namespace d3d
{
	class ShaderManager;
}

#include "d3d.h"

class DrawQuadRenderer
{
	d3d::Object<ID3D12RootSignature> m_rootSignature;
	d3d::Object<ID3D12PipelineState> m_pipelineState;

	bool loadShaders(d3d::ShaderManager* shaderManager);
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager);

public:
	DrawQuadRenderer();
	~DrawQuadRenderer();

	bool initialize(ID3D12Device* device, d3d::ShaderManager* shaderManager);

	void setPipelineState(ID3D12GraphicsCommandList* cmdList);
	void submitCommands(ID3D12GraphicsCommandList* cmdList);
};