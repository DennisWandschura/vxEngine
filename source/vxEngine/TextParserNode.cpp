#include "TextParserNode.h"

namespace TextParser
{
	const char* Node::searchForKey(const char* data)
	{
		auto i = 0u;
		auto c = data[i];
		while (c != ':' && c != '\0')
		{
			++i;
			c = data[i];
		}

		const char* ptr = nullptr;
		if (c != '\0')
		{
			ptr = data + i + 1;
		}

		return ptr;
	}

	const char* Node::createKeyAndGetDataBegin(const char* data)
	{
		auto end = searchForKey(data);
		if (end != nullptr)
		{
			m_key.append(data, end - 1);
		}

		return end + 1;
	}

	void Node::createData(const char* string)
	{
		auto i = 0u;
		auto c = string[i];

		m_isScalar = c != '[';

		while (c != '\0')
		{
			++i;
			c = string[i];
		}

		m_data.append(string, string + i);
	}

	Node::Node() :m_key(), m_data(), m_isScalar(true){}

	void Node::createNode(const std::string &string)
	{
		auto count = string.size();

		if (count != 0)
		{
			auto p = createKeyAndGetDataBegin(string.c_str());
			createData(p);
		}
	}

	const std::string& Node::getData() const
	{
		return m_data;
	}

	const std::string& Node::getKey() const
	{
		return m_key;
	}

	bool Node::isScalar() const
	{
		return m_isScalar;
	}
}