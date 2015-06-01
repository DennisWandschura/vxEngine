#pragma once
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

#include <vxLib/types.h>
#include <string>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>

namespace Parser
{
	template<typename T>
	struct Converter;

	class Node
	{
		std::string m_data;
		vx::sorted_vector<vx::StringID, Node> m_nodes;
		u32 m_dataSize;
		bool m_isArray;
		bool m_isMap;

		const char* insertNode(const char* str);

		const char* getArrayItemBegin(unsigned int i) const;

	public:
		Node();
		~Node();

		void create(const char* str);
		bool createFromFile(const char* file);

		const Node* get(const char* id) const;

		template<typename T>
		bool as(T* data) const
		{
			return Converter<T>::decode(*this, data);
		}

		bool as(bool* data) const;
		bool as(u8* data) const;
		bool as(s8* data) const;
		bool as(s32* data) const;
		bool as(u32* data) const;
		bool as(f32* data) const;
		bool as(f64* data) const;
		bool as(std::string* data) const;

		bool as(u32 i, int* data) const;
		bool as(u32 i, unsigned int* data) const;
		bool as(u32 i, float* data) const;
		bool as(u32 i, double* data) const;
		bool as(u32 i, std::string* data) const;
		bool as(unsigned int i, Node* data) const;

		bool isArray() const { return m_isArray; }
		u32 size() const { return m_dataSize; }
	};
}

#include "ParserConverter.h"