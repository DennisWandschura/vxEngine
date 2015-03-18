#include "Pool.h"

struct Freelist
{
	static const U16 s_magic = 0x1337;

	U16 nextFreeEntry;
	U16 magic;
};

void PoolBase::initialize(U8* ptr, U16 capacity, U32 chunkSize)
{
	assert(chunkSize >= sizeof(Freelist));

	m_ptr = ptr;
	m_capacity = capacity;

	U8* pCurrent = m_ptr;
	for (auto i = 0u; i < m_capacity; ++i)
	{
		Freelist* next = reinterpret_cast<Freelist*>(pCurrent);
		next->nextFreeEntry = i + 1;
		next->magic = Freelist::s_magic;

		pCurrent += chunkSize;
	}

	m_freeEntries = m_capacity;
}

U8* PoolBase::createEntry(U16* index, U32 chunkSize)
{
	if (m_freeEntries == 0)
		return nullptr;

	auto ptr = m_ptr + m_firstFreeEntry * (chunkSize);
	validateEmptyEntry(ptr);

	*index = m_firstFreeEntry;
	m_firstFreeEntry = reinterpret_cast<Freelist*>(ptr)->nextFreeEntry;
	--m_freeEntries;

	return ptr;
}

void PoolBase::destroyEntry(U8* ptr, U16 index)
{
	auto p = reinterpret_cast<Freelist*>(ptr);
	p->nextFreeEntry = m_firstFreeEntry;
	p->magic = Freelist::s_magic;

	m_firstFreeEntry = index;
	++m_freeEntries;
}

void PoolBase::validateEmptyEntry(const U8 *ptr)
{
	auto p = reinterpret_cast<const Freelist*>(ptr);
	VX_ASSERT(p->magic == Freelist::s_magic, "Entry not valid !");
}

void PoolBase::validateUsedEntry(const U8 *ptr)
{
	auto p = reinterpret_cast<const Freelist*>(ptr);
	VX_ASSERT(p->magic != Freelist::s_magic, "Entry not used !");
}

bool PoolBase::isUsed(const U8* ptr) const
{
	return (reinterpret_cast<const Freelist*>(ptr)->magic != Freelist::s_magic);
}

U8* PoolBase::release()
{
	auto p = m_ptr;

	m_ptr = nullptr;
	m_freeEntries = 0;
	m_capacity = 0;

	return p;
}

U16 PoolBase::size() const
{
	return m_capacity - m_freeEntries;
}

void* PoolBase::operator[](U16 i)
{
	return m_ptr + i;
}

const void* PoolBase::operator[](U16 i) const
{
	return m_ptr + i;
}

U8* PoolBase::first(U32 chunkSize) const
{
	auto end = m_ptr + m_capacity * chunkSize;
	U8* result = nullptr;
	U8* next = m_ptr;
	// skip unused entries
	while (next != end)
	{
		if (isUsed(next))
		{
			result = next;
			break;
		}

		next += chunkSize;
	}

	return result;
}

U8* PoolBase::next(U8* ptr, U32 chunkSize) const
{
	U8* next = ptr + chunkSize;
	auto end = m_ptr + m_capacity * chunkSize;

	U8* result = nullptr;
	// skip unused entries
	while (next != end)
	{
		if (isUsed(next))
		{
			result = next;
			break;
		}

		next += chunkSize;
	}

	return result;
}