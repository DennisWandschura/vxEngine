#pragma once

#include <vxLib/Variant.h>

enum class EventType : U8;

struct Event
{
	// type of event
	EventType type;
	// additional filter
	U16 filter;
	// specific event code of type
	U32 code;
	vx::Variant arg1;
	vx::Variant arg2;
};