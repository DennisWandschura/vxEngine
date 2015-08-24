#pragma once

struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;
struct ID3D12PipelineState;

#include "d3d.h"

class RenderPass
{
protected:
	d3d::Object<ID3D12RootSignature> m_rootSignature;
	d3d::Object<ID3D12PipelineState> m_pipelineState;

	RenderPass() :m_rootSignature(), m_pipelineState() {}

public:
	virtual ~RenderPass() {}

	virtual void submitCommands(ID3D12GraphicsCommandList* cmdList) = 0;
};