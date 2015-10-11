#include "GpuProfiler.h"
#include <d3d12.h>
#include "CommandList.h"
#include "d3dx12.h"
#include "ResourceManager.h"
#include "DownloadMananger.h"
#include <vxEngineLib/Graphics/CommandQueue.h>
#include "RenderAspect.h"
#include "FrameData.h"

struct GpuProfiler::Text
{
	char text[48];
};

struct GpuProfiler::Entry
{
	char text[48];
	u64 startTime;
	u64 endTime;
};

GpuProfiler::GpuProfiler()
	:m_currentCommandList(nullptr),
	m_data(),
	m_frequency(0),
	m_position(0, 0),
	m_currentFrame(0),
	m_countPerFrame(0),
	m_buildCommands(0)
{

}

GpuProfiler::~GpuProfiler()
{

}

void GpuProfiler::getRequiredMemory(u32 maxQueries, u64* bufferHeapSize, u32* bufferCount)
{
	auto sizeInBytes = d3d::getAlignedSize(maxQueries * sizeof(u64), 256llu) * 3 * 2;
	sizeInBytes = d3d::getAlignedSize(sizeInBytes, 64llu KBYTE);

	*bufferHeapSize += sizeInBytes;
	*bufferCount += 1;
}

bool GpuProfiler::initialize(u32 maxQueries, d3d::ResourceManager* resourceManager, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, const vx::float2 &position)
{
	D3D12_QUERY_HEAP_DESC desc;
	desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	desc.Count = maxQueries * 3 * 2;
	desc.NodeMask = 1;
	auto hr = device->CreateQueryHeap(&desc, IID_PPV_ARGS(m_queryHeap.getAddressOf()));
	if (hr != 0)
		return false;

	auto sizeInBytes = d3d::getAlignedSize(maxQueries * sizeof(u64), 256llu) * 3 * 2;
	sizeInBytes = d3d::getAlignedSize(sizeInBytes, 64llu KBYTE);
	auto buffer = resourceManager->createBuffer(L"queryBuffer", sizeInBytes, D3D12_RESOURCE_STATE_GENERIC_READ);
	if (buffer == nullptr)
		return false;

	if (!m_dlHeap.createBufferHeap(64 KBYTE, D3D12_HEAP_TYPE_READBACK, device))
		return false;

	d3d::HeapCreateBufferResourceDesc resDesc;
	resDesc.flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.resource = m_dlBuffer.getAddressOf();
	resDesc.size = 64 KBYTE;
	resDesc.state = D3D12_RESOURCE_STATE_COPY_DEST;
	if (!m_dlHeap.createResourceBuffer(resDesc))
		return false;

	m_countPerFrame = maxQueries;

	for (u32 i = 0; i < 3; ++i)
	{
		m_data[i].m_entries = std::make_unique<Text[]>(maxQueries);
		m_data[i].m_times = std::make_unique<u64[]>(maxQueries * 2);
		m_data[i].m_count = 0;
	}

	m_entries = std::make_unique<Entry[]>(maxQueries);
	m_sortedEntries.reserve(maxQueries);
	m_entryCount = 0;

	cmdQueue->GetTimestampFrequency(&m_frequency);
	m_position = position;

	return true;
}

void GpuProfiler::shutdown()
{
	m_queryHeap.destroy();
}

void GpuProfiler::frame(d3d::ResourceManager* resourceManager, FrameData* currentFrameData)
{
	auto queryFrame = m_currentFrame - 3;
	if (queryFrame > 0)
	{
		auto buffer = resourceManager->getBuffer(L"queryBuffer");

		auto queryBuffer = queryFrame % 3;

		auto &data = m_data[queryBuffer];
		auto &count = data.m_count;

		if (count != 0)
		{
			auto &allocator = currentFrameData->m_allocatorProfiler;
			auto &commandList = currentFrameData->m_commandListProfiler;

			allocator->Reset();
			commandList->Reset(allocator.get(), nullptr);
			auto bufferOffset = d3d::getAlignedSize((u64)m_countPerFrame * sizeof(u64) * (u64)queryBuffer, 256llu);
			auto queryIndexOffset = queryBuffer * m_countPerFrame;

			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer->get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
			commandList->ResolveQueryData(m_queryHeap.get(), D3D12_QUERY_TYPE_TIMESTAMP, queryIndexOffset, count, buffer->get(), bufferOffset);

			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer->get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

			auto sizeInBytes = count * sizeof(u64);
			commandList->CopyBufferRegion(m_dlBuffer.get(), bufferOffset, buffer->get(), bufferOffset, sizeInBytes);

			//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer->get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_GENERIC_READ));

			commandList->Close();

			m_currentCommandList = &commandList;
			m_buildCommands = 1;
		}
	}

	--queryFrame;
	if (queryFrame > 0)
	{
		auto queryBuffer = queryFrame % 3;

		auto &data = m_data[queryBuffer];

		if (data.m_count != 0)
		{
			auto sizeInBytes = data.m_count * sizeof(u64);
			auto bufferOffset = d3d::getAlignedSize((u64)m_countPerFrame * sizeof(u64) * (u64)queryBuffer, 256llu);

			D3D12_RANGE readRange;
			readRange.Begin = bufferOffset;
			readRange.End = bufferOffset + sizeInBytes;
			u8* gpuPtr = nullptr;
			m_dlBuffer->Map(0, &readRange, (void**)&gpuPtr);
			gpuPtr += bufferOffset;

			memcpy(data.m_times.get(), gpuPtr, sizeInBytes);
			m_dlBuffer->Unmap(0, nullptr);

			auto frequency = m_frequency;

			auto entryCount = data.m_count / 2;
			if (entryCount > m_sortedEntries.size())
			{
				m_sortedEntries.clear();
				m_entryCount = 0;
			}

			for (u32 timestampIndex = 0, entryIndex = 0; timestampIndex < data.m_count; timestampIndex += 2, ++entryIndex)
			{
				auto &entry = data.m_entries[entryIndex];

				auto startTime = data.m_times[timestampIndex];

				auto endTime = data.m_times[timestampIndex + 1];

				auto sid = vx::make_sid(entry.text);
				auto it = m_sortedEntries.find(sid);
				if (it == m_sortedEntries.end())
				{
					memcpy(m_entries[m_entryCount].text, entry.text, sizeof(entry.text));
					m_entries[m_entryCount].startTime = 0;
					m_entries[m_entryCount].endTime = 0;
					it = m_sortedEntries.insert(sid, m_entryCount++);
				}
				auto &dstEntry = m_entries[*it];
				dstEntry.startTime = startTime;
				dstEntry.endTime = endTime;

				data.m_times[timestampIndex] = 0;
				data.m_times[timestampIndex + 1] = 0;
			}

			data.m_count = 0;
		}
	}

	++m_currentFrame;
}

void GpuProfiler::queryBegin(const char* text, d3d::GraphicsCommandList* cmdList)
{
	auto currentBuffer = m_currentFrame % 3;

	auto &frameData = m_data[currentBuffer];
	auto &timeStampCount = frameData.m_count;
	VX_ASSERT(timeStampCount < m_countPerFrame);

	auto entryIndex = timeStampCount / 2;
	auto &entry = frameData.m_entries[entryIndex];
	sprintf(entry.text, "%s", text);

	auto offsetIndex = currentBuffer * m_countPerFrame;
	(*cmdList)->EndQuery(m_queryHeap.get(), D3D12_QUERY_TYPE_TIMESTAMP, offsetIndex + timeStampCount++);
}

void GpuProfiler::queryEnd(d3d::GraphicsCommandList* cmdList)
{
	auto currentBuffer = m_currentFrame % 3;

	auto &frameData = m_data[currentBuffer];
	auto &count = frameData.m_count;
	VX_ASSERT(count < m_countPerFrame);

	auto offsetIndex = currentBuffer * m_countPerFrame;
	(*cmdList)->EndQuery(m_queryHeap.get(), D3D12_QUERY_TYPE_TIMESTAMP, offsetIndex + count++);
}

void GpuProfiler::submitCommandList(Graphics::CommandQueue* queue)
{
	if (m_buildCommands != 0)
	{
		queue->pushCommandList(m_currentCommandList);
		m_buildCommands = 0;
	}
}

void GpuProfiler::update(RenderAspect* renderAspect)
{
	auto queryBuffer = (m_currentFrame +1) % 3;
	auto &data = m_data[queryBuffer];

	vx::float2 position = m_position;

	/*u32 entryIndex = 0;
	u32 i = 0;
	auto &startTime = data.m_times[i++];
	while (startTime != 0)
	{
		auto &endTime = data.m_times[i++];
		auto &entry = data.m_entries[entryIndex];

		auto sid = vx::make_sid(entry.text);
		auto it = m_sortedEntries.find(sid);
		if (it == m_sortedEntries.end())
		{
			memcpy(m_entries[m_entryCount].text, entry.text, sizeof(entry.text));
			m_entries[m_entryCount].startTime = 0;
			m_entries[m_entryCount].endTime = 0;
			it = m_sortedEntries.insert(sid, m_entryCount++);
		}

		auto &dstEntry = m_entries[*it];
		dstEntry.startTime = startTime;
		dstEntry.endTime = endTime;

		startTime = 0;
		endTime = 0;

		startTime = data.m_times[i++];
		++entryIndex;
	}*/

	const f32 yOffset = 15.f;

	auto pushUpdateText = [&yOffset](vx::float2* position, const char* text, RenderAspect* renderAspect)
	{
		RenderUpdateTextData updateData;
		updateData.position = *position;
		updateData.strSize = snprintf(updateData.text, 48, "%s", text);
		updateData.color = vx::float3(0, 1, 1);
		renderAspect->queueUpdate(RenderUpdateTaskType::UpdateText, (u8*)&updateData, sizeof(updateData));

		position->y -= yOffset;
	};

	auto pushUpdateTextTime = [&yOffset](vx::float2* position, const char* text, f64 timeMs, RenderAspect* renderAspect)
	{
		RenderUpdateTextData updateData;
		updateData.position = *position;
		updateData.strSize = snprintf(updateData.text, 48, "%s %.4f ms", text, timeMs);
		updateData.color = vx::float3(0, 1, 1);
		renderAspect->queueUpdate(RenderUpdateTaskType::UpdateText, (u8*)&updateData, sizeof(updateData));

		position->y -= yOffset;
	};

	if (m_entryCount != 0)
	{
		auto frequency = m_frequency;

		f64 totalTime = 0.0;
		f64 waitingTime = 0.0;

		auto &entry = m_entries[0];
		auto ticks = (entry.endTime - entry.startTime) * 1000;
		auto timeInMs = f64(ticks) / f64(frequency);

		totalTime += timeInMs;

		pushUpdateText(&position, "GPU: time, time to prev", renderAspect);
		pushUpdateTextTime(&position, entry.text, timeInMs, renderAspect);

		for (u32 i = 1; i < m_entryCount; ++i)
		{
			auto &lastEntry = m_entries[i - 1];
			auto entry = m_entries[i];

			auto ticks = (entry.endTime - entry.startTime) * 1000;
			auto timeInMs = f64(ticks) / f64(frequency);

			auto ticksBetweenEntries = (entry.startTime - lastEntry.endTime) * 1000;
			auto timeBetweenInMs = f64(ticksBetweenEntries) / f64(frequency);

			RenderUpdateTextData updateData;
			updateData.position = position;
			updateData.strSize = sprintf(updateData.text, "%s %.4f ms %.4f", entry.text, timeInMs, timeBetweenInMs);
			updateData.color = vx::float3(0, 1, 1);
			renderAspect->queueUpdate(RenderUpdateTaskType::UpdateText, (u8*)&updateData, sizeof(updateData));

			totalTime += timeInMs;
			waitingTime += timeBetweenInMs;

			position.y -= yOffset;
		}

		pushUpdateText(&position, "--------------------", renderAspect);
		pushUpdateTextTime(&position, "Total:", totalTime, renderAspect);
		pushUpdateTextTime(&position, "Waiting:", waitingTime, renderAspect);
	}
}