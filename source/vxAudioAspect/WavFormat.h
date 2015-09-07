#pragma once

#include <vxLib/types.h>

struct WavFormat
{
	u8 bytesPerSample;
	u8 channels;
	u16 formatTag;
};