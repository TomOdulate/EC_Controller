// 
// 
// 

#include "EventManager.h"
#include "RTC_DS3231.h"

/*  Event Manager Methods  */

EventManager::EventManager() : RTC_DS3231()
{

}

short EventManager::CountEvents()
{
	short count = 0;

	for(int i=0;i<100;i++)
	{
		if( this->EventData[i].EventTime != 0 )
			count++;
	}

	return count;
}

bool EventManager::AddEvent(DateTime time, short output, EventRepeatType repeatType, int interval, bool newState, EventType type)
{
	return this->AddEvent(time.unixtime(), output, repeatType, interval, newState, type);
}

bool EventManager::AddEvent(uint32_t time, short output, EventRepeatType repeatType, int interval, bool newState, EventType type)
{
	int nextAvailableIndex = this->FindSpace();
	
	if(nextAvailableIndex < 0)
		return false;
	else 
	{
		this->EventData[nextAvailableIndex].EventTime = time;
		this->EventData[nextAvailableIndex].Output = output;
		this->EventData[nextAvailableIndex].Repeat = repeatType;
		this->EventData[nextAvailableIndex].Type = type;
		this->EventData[nextAvailableIndex].newState = newState;
		this->EventData[nextAvailableIndex].Interval = interval;
		return true;
	}
}

bool EventManager::RemoveEvent(uint32_t time, short output, EventRepeatType repeatType)
{
	int foundEvent = FindEvent(time, output, repeatType);

	if(foundEvent < 0)
		return false;
	else 
	{
		return this->RemoveEvent(foundEvent);		
	}
}

bool EventManager::RemoveEvent(uint8_t index)
{
	this->EventData[index].EventTime = 0L;
	this->EventData[index].Output	= 0;
	this->EventData[index].Repeat	= ONESHOT;
	this->EventData[index].Type		= TRIGGER;
	this->EventData[index].newState = 0;
	return true;
}

int EventManager::FindSpace()
{
	// Return the first available unused index
	short index = -1;

	for(int i=0;i<100;i++)
	{
		if( this->EventData[i].EventTime == 0 )
		{
			index = i;
			break;
		}
	}

	return index;
}

int EventManager::FindEvent(uint32_t time, short output, EventRepeatType repeatType)
{
	short index = -1;

	for(int i=0;i<100;i++)
	{
		if( this->EventData[i].EventTime == time )
		{
			if( this->EventData[i].Output == OUTPUT )
			{
				if(this->EventData[i].Repeat == repeatType)
				{
					if(this->EventData[i].Type == TRIGGER)
					{
						index = i;
						break;
					}
				}
			}
		}
	}

	return index;
}

bool EventManager::Validate(uint32_t time, short output, EventRepeatType repeatType, int interval, bool newState, EventType type)
{
	// If the timestamp is invalid, stop
	if( time < 0 )
		return false;
	
	// If there is already an identical event, stop
	if( FindEvent(time, output, repeatType) > -1)
		return false;
	
	// Ok to add to list
	return this->AddEvent( time, output, repeatType, interval, newState, USER );
}

/*  Event Methods  */

Event::Event()
{
	// Just set some default values.  
	// Remember that the manager detects a null event as having a 0 eventtime.
	this->EventTime = 0L;
	this->Output = 0;
	this->Type = TRIGGER;
	this->Repeat = ONESHOT;
	this->newState = 0;
	this->Interval = 0;
}

Event::Event(uint32_t time, short output, EventRepeatType repeatType, int interval, bool NewState, EventType type)
{
	this->EventTime = time;
	this->Output	= output;	
	this->Repeat	= repeatType;
	this->Type		= type;
	this->newState	= NewState;
	this->Interval  = interval;
}

Event::Event(DateTime time, short output, EventRepeatType repeatType, int interval, bool NewState, EventType type)
{
	this->EventTime = time.unixtime();
	this->Output	= output;	
	this->Repeat	= repeatType;
	this->Type		= type;
	this->newState	= NewState;
	this->Interval  = interval;
}