#pragma once

namespace vx
{
	struct StringID;
}

#include "TextParserNodeConverter.h"
#include <vxLib/Container/sorted_vector.h>

namespace TextParser
{
	class File
	{
		vx::sorted_vector<vx::StringID, Node> m_nodes;

		bool isComment(const char* str);

		const char* getNextLine(const char* str, std::size_t* size);

		std::size_t countLines(const char* str);

		const char* getLine(const char* str, std::string* data);

	public:
		File();
		~File();

		void create(const char* buffer);
		bool createFromFile(const char* file);

		const Node* getNode(const char* id) const;
	};
}