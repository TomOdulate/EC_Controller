// EventManager.h
/****************************************************************************** 
	NAME   : EventManager.h
	Version: 0.2
	Created: 06/09/2013
	Author : Thomas Odulate
	NOTES  : Handles events, timings and provides a RTC using I2C.

******************************************************************************/
#ifndef _EVENTMANAGER_h
#define _EVENTMANAGER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "RTC_DS3231.h"
#include <Wire.h>

enum EventType { TRIGGER, USER };
enum EventRepeatType { ONESHOT = 0, SECONDS=1, MINUTE =  60, HOURLY = 3600, DAILY = 86400  };

class Event
{
	public:
		Event();
		Event(uint32_t time, short output, EventRepeatType repeatType, int interval, bool newState, EventType type);
		Event(DateTime time, short output, EventRepeatType repeatType, int interval, bool newState, EventType type);
		uint32_t EventTime;
		EventType Type;
		EventRepeatType Repeat;
		short Output;
		bool newState;
		int Interval;
};


class EventManager : public RTC_DS3231
{
	private:
		int FindSpace();
		int FindEvent(uint32_t time, short output, EventRepeatType repeatType);
		bool Validate(uint32_t time, short output, EventRepeatType repeatType, int interval, bool newState, EventType type);			// Validates a user created event
		
	public:
		EventManager();
		short CountEvents();
		bool AddEvent(uint32_t time, short output, EventRepeatType repeatType, int interval, bool NewState, EventType type);
		bool AddEvent(DateTime time, short output, EventRepeatType repeatType, int interval, bool NewState, EventType type);
		bool RemoveEvent(uint32_t time, short output, EventRepeatType repeatType);
		bool RemoveEvent(uint8_t index);
		Event EventData[100];
};

#endif
