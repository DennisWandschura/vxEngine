#pragma once

#include <vxLib/types.h>
#include "TextParserNode.h"
#include <vector>

namespace TextParser
{
	template<typename T>
	struct StringConverter;

	template<>
	struct StringConverter < float >
	{
		static float decode(const std::string &str)
		{
			return strtof(str.c_str(), nullptr);
		}
	};

	template<>
	struct StringConverter < int >
	{
		static int decode(const std::string &str)
		{
			return strtol(str.c_str(), nullptr, 10);
		}
	};

	template<>
	struct StringConverter < u32 >
	{
		static u32 decode(const std::string &str)
		{
			return strtoul(str.c_str(), nullptr, 10);
		}
	};

	template<>
	struct NodeConverter < float >
	{
		static float decode(const Node &node, f32 &data)
		{
			data = StringConverter<float>::decode(node.getData());
			return true;
		}
	};

	template<>
	struct NodeConverter < int >
	{
		static int decode(const Node &node, int &data)
		{
			data = StringConverter<int>::decode(node.getData());
			return true;
		}
	};

	template<>
	struct NodeConverter < bool >
	{
		static int decode(const Node &node, bool &data)
		{
			data = StringConverter<u32>::decode(node.getData());

			return true;
		}
	};

	template<>
	struct NodeConverter < u8 >
	{
		static int decode(const Node &node, u8 &data)
		{
			data = StringConverter<u32>::decode(node.getData());

			return true;
		}
	};

	template<>
	struct NodeConverter < u16 >
	{
		static int decode(const Node &node, u16 &data)
		{
			data = StringConverter<u32>::decode(node.getData());

			return true;
		}
	};

	template<>
	struct NodeConverter < u32 >
	{
		static int decode(const Node &node, u32 &data)
		{
			data = StringConverter<u32>::decode(node.getData());

			return true;
		}
	};

	template<>
	struct NodeConverter < std::string >
	{
		static int decode(const Node &node, std::string &data)
		{
			data = node.getData();

			return true;
		}
	};

	struct NodeConverterArray
	{
		static const char* getNext(const char* str)
		{
			auto i = 0u;
			auto c = str[0];

			while (c != ',' && c != '\0')
			{
				++i;
				c = str[i];
			}

			const char* ptr = nullptr;
			if (c != '\0')
			{
				ptr = str + i + 1;
			}

			return ptr;
		}

		static bool isValue(const char* ptr)
		{
			auto i = 0u;
			auto c = ptr[i];
			while (c == 32)
			{
				++i;
				c = ptr[i];
			}

			return(c != ']' && c != '\0');
		}

		static std::size_t getSize(const char* ptr)
		{
			std::size_t size = 0;

			auto next = getNext(ptr);
			while (next != nullptr)
			{
				if (isValue(ptr))
				{
					++size;
				}

				ptr = next;
				next = getNext(ptr);
			}

			if (isValue(ptr))
			{
				++size;
			}

			return size;
		}
	};

	template<typename T, size_t SIZE>
	struct NodeConverterArrayFixed : public NodeConverterArray
	{
		static bool decode(const Node &node, T(&data)[SIZE])
		{
			bool result = false;
			if (!node.isScalar())
			{
				auto &nodeData = node.getData();

				auto current = nodeData.c_str() + 1;
				auto size = getSize(current);
				if (size == SIZE)
				{
					auto i = 0u;

					auto current = nodeData.c_str() + 1;
					auto next = getNext(current);
					while (next != nullptr)
					{
						if (isValue(current))
						{
							data[i] = StringConverter<T>::decode(current);
							++i;
						}

						current = next;
						next = getNext(current);
					}

					if (isValue(current))
					{
						data[i] = StringConverter<T>::decode(current);
					}

					result = true;
				}
			}

			return result;
		}
	};

	template<typename T>
	struct NodeConverter < std::vector<T> > : public NodeConverterArray
	{
		static bool decode(const Node &node, std::vector<T>* data)
		{
			bool result = false;
			if (!node.isScalar())
			{
				auto &nodeData = node.getData();
				auto nodeDataSize = nodeData.size();

				auto current = nodeData.c_str() + 1;

				auto size = getSize(current);
				data->reserve(size);

				auto next = getNext(current);
				while (next != nullptr)
				{
					if (isValue(current))
					{
						auto value = StringConverter<T>::decode(current);
						data->push_back(value);
					}

					current = next;
					next = getNext(current);
				}

				if (isValue(current))
				{
					auto value = StringConverter<T>::decode(current);
					data->push_back(value);
				}

				result = true;
			}

			return result;
		}
	};
}