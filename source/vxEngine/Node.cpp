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

void Node::reserve(u32 n)
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