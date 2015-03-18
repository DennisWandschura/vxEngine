#ifndef __EVENTLISTENER_H
#define __EVENTLISTENER_H
#pragma once

struct Event;

class EventListener
{
protected:
	~EventListener(){}

public:
	virtual void handleEvent(const Event &evt) = 0;
};

#endif