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

#include <vector>

namespace vx
{
	template<typename T, typename Cmp>
	inline typename std::vector<T>::const_iterator vector_sortedInsert(std::vector<T>* vec, const T &value, Cmp cmp)
	{
		auto endKeys = vec->end();

		auto it = std::lower_bound(vec->begin(), vec->end(), value, cmp);

		auto index = it - vec->begin();
		if (it == vec->end() || cmp(value, *it))
		{
			auto _Off = it - vec->begin();

			vec->emplace_back(value);

			std::rotate(vec->begin() + _Off, vec->end() - 1, vec->end());
		}

		return vec->begin() + index;
	}

	template<typename T, typename Cmp>
	inline typename std::vector<T>::const_iterator vector_find(const std::vector<T> &vec, const T &value, Cmp cmp)
	{
		auto it = std::lower_bound(vec.begin(), vec.end(), value, cmp);
		auto index = it - vec.begin();

		auto result = vec.begin() + index;
		if (it != vec.end() && cmp(value, *it))
			result = vec.end();

		return result;
	}
}