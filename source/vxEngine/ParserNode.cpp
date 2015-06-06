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

#include "ParserNode.h"
#include <fstream>
#include <memory>

namespace Parser
{
	const char* getKeyBeginEnd(const char* str, int* startOffset)
	{
		while (str[0] == ' ' ||
			str[0] == '\n' ||
			str[0] == '{')
		{
			++str;
			++(*startOffset);
		}

		char c = str[0];

		int i = 0;
		while (true)
		{
			if (c == '\0')
				break;

			if (c == ':')
				break;

			++i;
			c = str[i];
		}

		return str + i;
	}

	const char* getDataBegin(const char* str)
	{
		char c = str[0];
		int i = 0;
		while (c == ' ' || c == ':' || c == '\n')
		{
			++i;
			c = str[i];
		}

		return str + i;
	}

	const char* getSpace(const char* str, int* size)
	{
		char c = str[0];
		int i = 0;
		while (c != ' ')
		{
			if (c == '\n')
				break;

			if (c == '\0')
				return nullptr;

			++(*size);
			++i;
			c = str[i];
		}

		return str + i;
	}

	const char* getArrayEnd(const char* str)
	{
		int layer = 1;

		while (true)
		{
			char c = str[0];

			if (c == '\0')
				break;

			if (c == '[')
				++layer;

			if (c == ']')
				--layer;

			if (layer == 0)
				break;

			++str;
		}

		return str;
	}

	const char* getBracesEnd(const char* str, int layer)
	{
		while (true)
		{
			char c = str[0];
			if (c == '{')
			{
				++layer;
			}

			if (c == '}')
			{
				--layer;
				if (layer == 0)
					break;
			}

			++str;
		}

		return str;
	}

	unsigned int countArrayData(const char* str)
	{
		unsigned int count = 0;

		int layer = 0;

		if (str[0] != '\0')
			count = 1;

		while (true)
		{
			if (str[0] == '\0')
				break;

			if (str[0] == '[')
				++layer;

			if (str[0] == ']')
				--layer;

			if (str[0] == ',' && layer == 0)
				++count;

			++str;
		}

		return count;
	}

	const char* skipOpenBracketAndSpace(const char* str)
	{
		while (str[0] == '[' ||
			str[0] == ' ')
		{
			if (str[0] == '\0')
				break;

			++str;
		}

		return str;
	}

	const char* skipSpace(const char* str)
	{
		while (str[0] == ' ')
		{
			if (str[0] == '\0')
				break;

			++str;
		}

		return str;
	}

	Node::Node()
		:m_data(),
		m_nodes(),
		m_dataSize(0),
		m_isArray(false),
		m_isMap(false)
	{

	}

	Node::Node(Node &&rhs)
		:m_data(std::move(rhs.m_data)),
		m_nodes(std::move(rhs.m_nodes)),
		m_dataSize(rhs.m_dataSize),
		m_isArray(rhs.m_isArray),
		m_isMap(rhs.m_isMap)
	{

	}

	Node::~Node()
	{

	}

	Node& Node::operator = (Node &&rhs)
	{
		if (this != &rhs)
		{
			m_data = std::move(rhs.m_data);
			m_nodes = std::move(rhs.m_nodes);
			m_dataSize = rhs.m_dataSize;
			m_isArray = rhs.m_isArray;
			m_isMap = rhs.m_isMap;
		}

		return *this;
	}

	const char* Node::insertNode(const char* str)
	{
		if (str == nullptr)
			return nullptr;

		int offset = 0;
		auto keyEnd = getKeyBeginEnd(str, &offset);
		str += offset;

		if (str[0] == '}')
		{
			++str;

			offset = 0;
			keyEnd = getKeyBeginEnd(str, &offset);
			str += offset;

		}

		if (str[0] == '\0')
			return nullptr;

		std::string key;
		key.append(str, keyEnd);

		auto diff = keyEnd - str;

		auto dataBegin = getDataBegin(str + diff);

		const char* next = nullptr;
		std::string data;
		bool isArray = false;
		bool isMap = false;

		Node n;

		if (dataBegin[0] == '[')
		{
			dataBegin = skipOpenBracketAndSpace(dataBegin);
			//++dataBegin;
			//++dataBegin;
			auto dataEnd = getArrayEnd(dataBegin);
			data.append(dataBegin, dataEnd);

			next = dataEnd + 1;
			isArray = true;

			n.m_dataSize = countArrayData(data.c_str());

			n.m_data = data;
		}
		else if (dataBegin[0] == '{')
		{
			dataBegin = skipSpace(++dataBegin);

			int layer = 1;
			auto dataEnd = getBracesEnd(dataBegin, layer);
			next = dataEnd + 1;
			--dataEnd;
			data.append(dataBegin, dataEnd);

			n.create(data.c_str());

			isMap = true;
		}
		else
		{
			int offset = 0;
			next = getSpace(dataBegin, &offset);

			auto dataEnd = dataBegin + offset;
			data.append(dataBegin, dataEnd);

			n.m_data = data;
			n.m_dataSize = 1;
		}


		n.m_isArray = isArray;
		n.m_isMap = isMap;

		auto sid = vx::make_sid(key.c_str());
		m_nodes.insert(std::move(sid), std::move(n));

		if (next && next[0] == '\0')
			next = nullptr;

		return next;
	}

	void Node::create(const char* str)
	{
		while (str != nullptr)
			str = insertNode(str);
	}

	bool Node::createFromFile(const char* file)
	{
		std::ifstream inFile(file);
		if (!inFile.is_open())
			return false;

		inFile.seekg(0, std::ifstream::end);
		auto sz = inFile.tellg();
		inFile.seekg(0, std::ifstream::beg);

		auto data = std::make_unique<char[]>(sz);

		inFile.read(data.get(), sz);

		create(data.get());

		return true;
	}

	const Node* Node::get(const char* id) const
	{
		const Node* result = nullptr;
		auto sid = vx::make_sid(id);

		auto it = m_nodes.find(sid);
		if (it != m_nodes.end())
		{
			result = &(*it);
		}

		return result;
	}

	bool Node::as(bool* data) const
	{
		*data = strtol(m_data.c_str(), nullptr, 10);
		return true;
	}

	bool Node::as(u8* data) const
	{
		*data = strtoul(m_data.c_str(), nullptr, 10);
		return true;
	}

	bool Node::as(s8* data) const
	{
		*data = strtol(m_data.c_str(), nullptr, 10);
		return true;
	}

	bool Node::as(int* data) const
	{
		*data = strtol(m_data.c_str(), nullptr, 10);
		return true;
	}

	bool Node::as(unsigned int* data) const
	{
		*data = strtoul(m_data.c_str(), nullptr, 10);
		return true;
	}

	bool Node::as(float* data) const
	{
		*data = strtof(m_data.c_str(), nullptr);
		return true;
	}

	bool Node::as(double* data) const
	{
		*data = strtod(m_data.c_str(), nullptr);
		return true;
	}

	bool Node::as(std::string* data) const
	{
		*data = m_data;
		return true;
	}

	const char* getArrayDataBegin(unsigned i, const char* str)
	{
		int layer = 0;

		while (i != 0)
		{
			if (str[0] == '\0')
				return nullptr;

			if (str[0] == '[')
				++layer;

			if (str[0] == ']')
				--layer;

			if (str[0] == ',' && layer == 0)
				--i;

			++str;
		}

		while (str[0] == ' ')
			++str;

		return str;
	}

	const char* getArrayDataEnd(const char* str)
	{
		int layer = 0;

		while (true)
		{
			if (str[0] == '\0')
				break;

			if (str[0] == '[')
				++layer;

			if (str[0] == ']')
				--layer;

			if (str[0] == ',' && layer == 0)
				break;

			++str;
		}

		return str;
	}

	const char* Node::getArrayItemBegin(unsigned int i) const
	{
		if (i >= m_dataSize)
			return nullptr;

		return getArrayDataBegin(i, m_data.c_str());
	}

	bool Node::as(unsigned int i, int* data) const
	{
		if (i >= m_dataSize)
			return false;

		auto begin = getArrayItemBegin(i);
		if (begin == nullptr)
			return false;

		*data = strtol(begin, nullptr, 10);
		return true;
	}

	bool Node::as(unsigned int i, unsigned int* data) const
	{
		if (i >= m_dataSize)
			return false;

		auto begin = getArrayItemBegin(i);
		if (begin == nullptr)
			return false;

		*data = strtoul(begin, nullptr, 10);
		return true;
	}

	bool Node::as(unsigned int i, float* data) const
	{
		if (i >= m_dataSize)
			return false;

		auto begin = getArrayItemBegin(i);
		if (begin == nullptr)
			return false;

		*data = strtof(begin, nullptr);
		return true;
	}

	bool Node::as(unsigned int i, double* data) const
	{
		if (i >= m_dataSize)
			return false;

		auto begin = getArrayItemBegin(i);
		if (begin == nullptr)
			return false;

		*data = strtod(begin, nullptr);
		return true;
	}

	bool Node::as(u32 i, std::string* data) const
	{
		if (i >= m_dataSize)
			return false;

		auto begin = getArrayItemBegin(i);
		if (begin == nullptr)
			return false;

		auto end = getArrayDataEnd(begin);

		if (i == m_dataSize - 1)
		{
			if (end[0] == '\0' && end[-1] == ' ')
				--end;
		}

		data->append(begin, end);
		return true;
	}

	bool Node::as(unsigned int i, Node* data) const
	{
		std::string tmp;
		if (!as(i, &tmp))
			return false;

		data->create(tmp.c_str());

		return true;
	}
}