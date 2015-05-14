#include "TextParserFile.h"
#include <fstream>
#include <vxLib/memory.h>
#include <vxLib/StringID.h>

namespace TextParser
{
	File::File()
		:m_nodes()
	{

	}

	File::~File()
	{

	}

	bool File::isComment(const char* str)
	{
		auto c = str[0];

		if (c == '\0')
			return false;

		auto d = str[1];

		return (c == '/') && (d == '/');
	}

	const char* File::getNextLine(const char* str, std::size_t* size)
	{

		auto i = 0u;
		auto c = str[i];
		while (c != '\n' && c != '\0')
		{
			++i;
			c = str[i];
		}

		*size = i;

		const char* p = nullptr;
		if (c == '\n')
		{
			++i;
			p = str + i;
		}

		return p;
	}

	std::size_t File::countLines(const char* str)
	{
		std::size_t count = 0;

		auto i = 0u;
		auto c = str[i];
		while (c != '\n' && c != '\0')
		{
			++i;
			c = str[i];
		}

		return count;
	}

	const char* File::getLine(const char* str, std::string* data)
	{
		bool comment = isComment(str);

		std::size_t lineSize = 0;
		auto p = getNextLine(str, &lineSize);

		if (!comment && str != nullptr)
		{
			data->append(str, str + lineSize);
		}

		return p;
	}

	void File::create(const char* buffer)
	{
		std::string data;
		auto ptr = getLine(buffer, &data);
		if (data.size() != 0)
		{
			Node n;
			n.createNode(data);

			auto sid = vx::make_sid(n.getKey().c_str());

			m_nodes.insert(sid, n);
		}
		data.clear();

		while (ptr != nullptr)
		{
			ptr = getLine(ptr, &data);
			if (data.size() != 0)
			{
				Node n;
				n.createNode(data);
				auto sid = vx::make_sid(n.getKey().c_str());

				m_nodes.insert(sid, n);
			}
			data.clear();
		}
	}

	bool File::createFromFile(const char* file)
	{
		std::ifstream infile(file);
		if (!infile.is_open())
		{
			return false;
		}

		infile.seekg(0, infile.end);
		auto count = infile.tellg();
		infile.seekg(0, infile.beg);

		auto buffer = vx::make_unique<char[]>(count);
		infile.read(buffer.get(), count);
		infile.close();

		create(buffer.get());

		return true;
	}

	const Node* File::getNode(const char* id) const
	{
		const Node* p = nullptr;

		auto sid = vx::make_sid(id);

		auto it = m_nodes.find(sid);
		if (it != m_nodes.end())
		{
			p = &*it;
		}

		return p;
	}
}