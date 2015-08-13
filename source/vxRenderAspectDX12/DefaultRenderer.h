#pragma once

struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D10Blob;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;

typedef ID3D10Blob ID3DBlob;

#include "d3d.h"

class DefaultRenderer
{
	d3d::Object<ID3D12RootSignature> m_rootSignature;
	d3d::Object<ID3D12PipelineState> m_pipelineState;
	d3d::Object<ID3DBlob> m_shaders[2];

	bool loadShaders();
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device);

public:
	DefaultRenderer();
	~DefaultRenderer();

	bool initialize(ID3D12Device* device);

	void submitCommands(ID3D12GraphicsCommandList* cmdList);
};