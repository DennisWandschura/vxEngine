#pragma once

#include <vxLib/types.h>

namespace d3d
{
	template<typename T>
	class Object
	{
	protected:
		T* m_object;

	public:
		Object() :m_object(nullptr) {}
		~Object()
		{
			destroy();
		}

		void destroy()
		{
			if (m_object)
			{
				m_object->Release();
				m_object = nullptr;
			}
		}

		T* operator->() { return m_object; }
		const T* operator->() const { return m_object; }

		T* get() { return m_object; }
		const T* get() const { return m_object; }

		T** getAddressOf() { return &m_object; }
		const T** getAddressOf() const { return &m_object; }
	};

	inline u32 getAlignedSize(u32 size, u32 alignment)
	{
		alignment = alignment - 1;
		return (size + alignment) & ~alignment;
	}

	inline u64 getAlignedSize(u64 size, u64 alignment)
	{
		alignment = alignment - 1;
		return (size + alignment) & ~alignment;
	}
}