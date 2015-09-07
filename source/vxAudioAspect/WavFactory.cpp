#include "WavFactory.h"
#include <vxLib/File/File.h>
#include "AudioFormat.h"
#include "WavFile.h"
#include "WavFormat.h"

bool WavFactory::loadFromFile(const char* filepath, WavFile* wav, WavFormat* format)
{
	vx::File file;
	if (!file.open(filepath, vx::FileAccess::Read))
		return false;

	Wav::Header header;
	file.read(header);

	if (!header.isValid())
		return false;

	Wav::FormatHeader formatHeader;
	file.read(formatHeader);

	format->channels = formatHeader.channels;
	format->bytesPerSample = formatHeader.bitsPerSample / 8;
	format->formatTag = formatHeader.formatTag;

	while (true)
	{
		u8 dataTag[4];

		file.read(dataTag, 4);

		if (dataTag[0] == 'd' &&
			dataTag[1] == 'a' &&
			dataTag[2] == 't' &&
			dataTag[3] == 'a')
			break;
	}

	u32 dataSize = 0;
	file.read(dataSize);

	u8* data = new u8[dataSize];
	file.read(data, dataSize);

	wav->create(data, dataSize);

/*	file->bufferSize = dataSize;
	file->format = formatData;
	file->frames = dataSize / 2;
	file->m_channels = formatData.channels;
	file->m_size = formatData.bits_per_sample / 8;*/

	return true;
}