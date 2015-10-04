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

#include "Debug.h"
#include <Initguid.h>
#include <dxgidebug.h>
#include <d3d12.h>
#include <cstdio>
#include <vxEngineLib/Logfile.h>

namespace d3d
{
	typedef HRESULT(WINAPI *DXGIGetDebugInterfaceProc)(REFIID riid, void **ppDebug);

	Debug::Debug()
		:m_dxgidebugDllHandle(nullptr)
	{

	}

	Debug::~Debug()
	{

	}

	bool Debug::getDebugInterface()
	{
		auto hresult = D3D12GetDebugInterface(IID_PPV_ARGS(m_debug.getAddressOf()));
		if (hresult != 0)
			return false;

		m_debug->EnableDebugLayer();

		return true;
	}

	bool Debug::initialize(vx::StackAllocator* allocator, ID3D12Device* device, Logfile* errorLog)
	{
		m_errorLog = errorLog;

		const u32 scratchAllocSize = 10 KBYTE;
		auto scratchAllocPtr = allocator->allocate(scratchAllocSize);
		VX_ASSERT(scratchAllocPtr);
		m_scratchAllocator = vx::StackAllocator(scratchAllocPtr, scratchAllocSize);

		auto hresult = device->QueryInterface(IID_PPV_ARGS(m_infoQueue.getAddressOf()));
		if (hresult != 0)
		{
			return false;
		}

		m_dxgidebugDllHandle = GetModuleHandle(L"Dxgidebug.dll");
		if (m_dxgidebugDllHandle == nullptr)
			return false;

		auto dXGIGetDebugInterfaceProc = (DXGIGetDebugInterfaceProc)GetProcAddress((HMODULE)m_dxgidebugDllHandle, "DXGIGetDebugInterface");
		if (dXGIGetDebugInterfaceProc == nullptr)
			return false;

		hresult = dXGIGetDebugInterfaceProc(IID_PPV_ARGS(m_dxgiDebug.getAddressOf()));
		if (hresult != 0)
			return false;

		hresult = dXGIGetDebugInterfaceProc(IID_PPV_ARGS(m_dxgiInfoQueue.getAddressOf()));
		if (hresult != 0)
			return false;

		return true;
	}

	void Debug::shutdownDevice()
	{
		m_infoQueue.destroy();
	}

	void Debug::shutdown()
	{
		m_dxgiInfoQueue.destroy();
		m_dxgiDebug.destroy();

		if (m_dxgidebugDllHandle)
		{
			FreeLibrary((HMODULE)m_dxgidebugDllHandle);
			m_dxgidebugDllHandle = nullptr;
		}

		
		m_debug.destroy();

		m_scratchAllocator.release();
	}

	void Debug::printDebugMessages()
	{
		if (m_infoQueue.get())
		{
			auto msgCount = m_infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
			for (u64 i = 0; i < msgCount; ++i)
			{
				size_t msgSize = 0;
				auto hresult = m_infoQueue->GetMessage(i, nullptr, &msgSize);

				auto marker = m_scratchAllocator.getMarker();
				D3D12_MESSAGE* msg = (D3D12_MESSAGE*)m_scratchAllocator.allocate(msgSize, 8);

				hresult = m_infoQueue->GetMessage(i, msg, &msgSize);

				printf("%s\n", msg->pDescription);

				if (msg->Severity == D3D12_MESSAGE_SEVERITY_ERROR)
				{
					m_errorLog->append(msg->pDescription, msg->DescriptionByteLength - 1);
					m_errorLog->append('\n');
					VX_ASSERT(false);
				}

				m_scratchAllocator.clear(marker);
			}
			m_infoQueue->ClearStoredMessages();
		}
	}

	void Debug::reportLiveObjects()
	{
		auto hresult = m_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);

		auto count = m_dxgiInfoQueue->GetNumStoredMessagesAllowedByRetrievalFilters(DXGI_DEBUG_ALL);
		if (count != 0)
		{
			m_errorLog->append("\nLive Objects\n");
			for (u64 i = 0; i < count; ++i)
			{
				auto marker = m_scratchAllocator.getMarker();

				size_t msgSize = 0;
				m_dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &msgSize);

				DXGI_INFO_QUEUE_MESSAGE* ptr = (DXGI_INFO_QUEUE_MESSAGE*)m_scratchAllocator.allocate(msgSize);

				m_dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, ptr, &msgSize);

				//printf("%s\n", ptr->pDescription);
				m_errorLog->append(ptr->pDescription, ptr->DescriptionByteLength - 1);
				m_errorLog->append('\n');

				m_scratchAllocator.clear(marker);
			}
		}
	}
}