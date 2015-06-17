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
#include <vxEngineLib/FileFactory.h>
#include <vxLib/File/FileHeader.h>
#include <vxLib/File/File.h>

namespace vx
{
	void FileFactory::saveToFile(const char* filename, const Serializable* data)
	{
		vx::File f;
		f.create(filename, vx::FileAccess::Write);

		saveToFile(&f, data);
	}

	void FileFactory::saveToFile(File* f, const Serializable* data)
	{
		FileHeader header;

		header.magic = FileHeader::s_magic;
		header.version = data->getVersion();
		header.crc = data->getCrc();

		f->write(header);
		data->saveToFile(f);
		f->write(header);
	}

	bool FileFactory::validate(const char* filename)
	{
		vx::File f;
		if (!f.open(filename, vx::FileAccess::Read))
			return false;

		FileHeader headerTop;
		if (!f.read(headerTop))
			return false;

		if (!f.setEof())
			return false;

		s32 offset = -static_cast<s64>(sizeof(FileHeader));
		f.seek(offset);

		FileHeader headerBottom;
		if (!f.read(headerBottom))
			return false;

		return headerBottom.isEqual(headerTop);
	}
}