#pragma once

struct ID3D12Resource;

#include "Buffer.h"

struct MappedBuffer
{
	void* ptr;
	u64 start;
	u64 end;

	MappedBuffer() :ptr(nullptr), start(0), end(0) {}
	explicit MappedBuffer(u64 size) :ptr(nullptr), start(0), end(size) {}
	MappedBuffer(u64 offset, u64 size) :ptr(nullptr), start(offset), end(offset + size) {}
};

class UploadBuffer
{
	ID3D12Resource* m_buffer;
	u32 m_size;

public:
	UploadBuffer();
	UploadBuffer(ID3D12Resource* buffer, u32 size);
	UploadBuffer(const UploadBuffer&) = delete;
	UploadBuffer(UploadBuffer &&rhs);
	~UploadBuffer();

	UploadBuffer& operator=(const UploadBuffer&) = delete;
	UploadBuffer& operator=(UploadBuffer &&rhs);

	void swap(UploadBuffer &other);

	void release();

	bool isValid() const;

	s32 map(MappedBuffer* mappedBuffer);

	void unmap(MappedBuffer* mappedBuffer);
};
