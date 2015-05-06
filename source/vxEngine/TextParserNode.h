#pragma once

#include <string>

namespace TextParser
{
	template<typename T>
	struct NodeConverter;

	template<typename T, size_t SIZE>
	struct NodeConverterArrayFixed;

	class Node
	{
		std::string m_key;
		std::string m_data;
		bool m_isScalar;

		const char* searchForKey(const char* data);
		const char* createKeyAndGetDataBegin(const char* data);
		void createData(const char* string);

	public:
		Node();

		void createNode(const std::string &string);

		template<typename T, typename std::enable_if<!std::is_array<T>::value>::type* = nullptr>
		bool as(T &data) const
		{
			return NodeConverter<T>::decode(*this, data);
		}

		template<typename T, size_t SIZE>
		bool as(T(&data)[SIZE]) const
		{
			return NodeConverterArrayFixed<T, SIZE>::decode(*this, data);
		}

		const std::string& getData() const;

		const std::string& getKey() const;

		bool isScalar() const;
	};
}