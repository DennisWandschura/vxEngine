#pragma once

#include "Variant.h"

enum class EventType : U8;

struct Event
{
	// type of event
	EventType type;
	// additional filter
	U16 filter;
	// specific event code of type
	U32 code;
	Variant arg1;
	Variant arg2;
};