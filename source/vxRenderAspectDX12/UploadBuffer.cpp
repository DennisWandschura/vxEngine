#include "UploadBuffer.h"
#include <d3d12.h>

const s64 g_bufferAlignment{ 0xffff + 1 };

UploadBuffer::UploadBuffer()
	:m_buffer(nullptr),
	m_size(0)
{

}

UploadBuffer::UploadBuffer(UploadBuffer &&rhs)
:m_buffer(rhs.m_buffer),
m_size(rhs.m_size)
{
	rhs.m_buffer = nullptr;
	rhs.m_size = 0;
}

UploadBuffer::UploadBuffer(ID3D12Resource* buffer, u32 size)
	:m_buffer(buffer),
	m_size(size)
{

}

UploadBuffer::~UploadBuffer()
{
	release();
}

UploadBuffer& UploadBuffer::operator=(UploadBuffer &&rhs)
{
	if (this != &rhs)
	{
		swap(rhs);
	}

	return *this;
}

void UploadBuffer::swap(UploadBuffer &other)
{
	auto tmp = m_buffer;
	m_buffer = other.m_buffer;
	other.m_buffer = tmp;

	auto tmpSize = m_size;
	m_size = other.m_size;
	other.m_size = tmpSize;
}

void UploadBuffer::release()
{
	if (m_buffer)
	{
		m_buffer->Release();
		m_buffer = nullptr;
	}
}

s32 UploadBuffer::map(MappedBuffer* mappedBuffer)
{
	D3D12_RANGE range{ mappedBuffer->start, mappedBuffer->end };
	return m_buffer->Map(0, &range, &mappedBuffer->ptr);
}

void UploadBuffer::unmap(MappedBuffer* mappedBuffer)
{
	D3D12_RANGE range{ mappedBuffer->start, mappedBuffer->end };
	m_buffer->Unmap(0, &range);

	mappedBuffer->ptr = nullptr;
	mappedBuffer->start = 0;
	mappedBuffer->end = 0;
}

bool UploadBuffer::isValid() const
{
	return (m_buffer != nullptr);
}