#pragma once

#include <vxLib/types.h>
#include <atomic>

class atomic_float
{
	std::atomic_int m_data;

public:
	atomic_float() :m_data() {}

	void store(f32 v)
	{
		s32* p = (s32*)&v;
		m_data.store(*p);
	}

	f32 load()
	{
		auto v = m_data.load();
		return *((f32*)&v);
	}

	f32 fetch_add(f32 v)
	{
		s32 oldValue = m_data.load();
		float newValue;

		do
		{
			newValue = *((f32*)&oldValue) + v;
		} while (!m_data.compare_exchange_strong(oldValue, *((s32*)&newValue)));

		return *((f32*)&oldValue);
	}

	f32 fetch_sub(f32 v)
	{
		s32 oldValue = m_data.load();
		float newValue;

		do
		{
			newValue = *((f32*)&oldValue) - v;
		} while (!m_data.compare_exchange_strong(oldValue, *((s32*)&newValue)));

		return *((f32*)&oldValue);
	}

	bool compare_exchange_strong(f32* expected, f32 value)
	{
		return m_data.compare_exchange_strong(*((s32*)expected), *((s32*)&value));
	}
};