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
#include <map>

namespace Parser
{
	template<typename T>
	struct Converter;

	class Node
	{
		std::string m_data;
		std::map<std::string, Node> m_nodes;
		unsigned int m_dataSize;
		bool m_isArray;
		bool m_isMap;

		const char* insertNode(const char* str);

		const char* getArrayItemBegin(unsigned int i) const;

	public:
		Node();
		~Node();

		void create(const char* str);
		void createFromFile(const char* file);

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

		bool as(unsigned int i, int* data) const;
		bool as(unsigned int i, unsigned int* data) const;
		bool as(unsigned int i, float* data) const;
		bool as(unsigned int i, double* data) const;

		bool isArray() const { return m_isArray; }
		unsigned int size() const { return m_dataSize; }
	};
}

#include "ParserConverter.h"