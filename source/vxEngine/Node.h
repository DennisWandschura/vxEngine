#pragma once

#include <vxLib\types.h>
#include "rtti.h"
#include <vxLib\Container\sorted_vector.h>
#include <vxLib\StringID.h>

class NodeData
{
	void *m_ptr;
	const rtti::TypeData *m_pTypeData;

public:
	NodeData();
	NodeData(void *ptr, const rtti::TypeData *pTypeData);
	NodeData(NodeData &&rhs);

	~NodeData();

	NodeData& operator=(NodeData &&rhs);

	void* get();

	template<class T>
	T* get()
	{
		if (*rtti::TypeCreator<T>::getTypeData() != *m_pTypeData)
		{
			VX_ASSERT(false);
			return nullptr;
		}

		return reinterpret_cast<T*>(m_ptr);
	}

	const rtti::TypeData* getTypeInfo() const{ return m_pTypeData; }
};

class Node
{
	vx::sorted_vector<vx::StringID, NodeData*> m_data;

	void addData(void *ptr, const rtti::TypeData *pTypeData, vx::StringID sid);

public:
	Node();
	Node(const Node&) = delete;
	Node(Node &&rhs);

	Node& operator=(const Node&) = delete;
	Node& operator=(Node &&rhs);

	void reserve(U32 n);

	template<class T, typename = typename std::enable_if_t<!std::is_pointer<T>::value>>
	void addData(const char *id, const T &value)
	{
		T *ptr = new T(value);
		auto pTypeData = rtti::SingletonRTTI::get().getTypeData<T>();

		addData(ptr, pTypeData, vx::StringID(id));
	}

	template<class T, typename = typename std::enable_if_t<!std::is_pointer<T>::value>>
	void addData(const char *id, T &&value)
	{
		typedef typename std::decay_t<T> value_type;

		value_type *ptr = new value_type(std::forward<value_type>(value));
		auto pTypeData = rtti::SingletonRTTI::get().getTypeData<value_type>();

		addData(ptr, pTypeData, vx::StringID(id));
	}

	template<class T> T* get(const char *id)
	{
		return get<T>(vx::StringID(id));
	}

	template<class T> T* get(const vx::StringID sid)
	{
		auto it = m_data.find(sid);
		if (it == m_data.end())
			return nullptr;

		return (*it)->get<T>();
	}

	template<class T> const T* get(const char *id) const
	{
		return get<T>(vx::StringID(id));
	}

	template<class T> const T* get(const vx::StringID sid) const
	{
		auto it = m_data.find(sid);
		if (it == m_data.end())
			return nullptr;

		return (*it)->get<T>();
	}

	void erase(const char *id);
	void erase(const vx::StringID sid);
};