#include "Node.h"
#include <vxLib\util\CityHash.h>

NodeData::NodeData() :m_ptr(nullptr), m_pTypeData(nullptr){}

NodeData::NodeData(void *ptr, const rtti::TypeData *pTypeData)
	: m_ptr(ptr),
	m_pTypeData(pTypeData)
{
}

NodeData::NodeData(NodeData &&rhs)
	: m_ptr(rhs.m_ptr),
	m_pTypeData(rhs.m_pTypeData)
{
	rhs.m_ptr = nullptr;
}

NodeData::~NodeData()
{
	if (m_ptr)
	{
		rtti::SingletonRTTI::get().destroy(m_pTypeData, m_ptr);
		m_ptr = nullptr;
		printf("aaaa\n");
	}
}

NodeData& NodeData::operator=(NodeData &&rhs)
{
	if (this != &rhs)
	{
		m_ptr = rhs.m_ptr;
		m_pTypeData = rhs.m_pTypeData;
		rhs.m_ptr = nullptr;
	}
	return *this;
}

void* NodeData::get()
{
	return m_ptr;
}

Node::Node()
	:m_data()
{
}

Node::Node(Node &&rhs)
	: m_data(std::move(rhs.m_data))
{
}

Node& Node::operator = (Node &&rhs)
{
	if (this != &rhs)
	{
		m_data = std::move(rhs.m_data);
	}
	return *this;
}

void Node::reserve(U32 n)
{
	m_data.reserve(n);
}

void Node::addData(void *ptr, const rtti::TypeData *pTypeData, vx::StringID sid)
{
	NodeData *pData = new NodeData(ptr, pTypeData);

	m_data.insert(sid, pData);
}

void Node::erase(const char *id)
{
	erase(vx::make_sid(id));
}

void Node::erase(const vx::StringID sid)
{
	auto it = m_data.find(sid);
	if (it != m_data.end())
	{
		m_data.erase(it);
	}
}