// SDLogger.h
/****************************************************************************** 
	NAME   : SDLogger.h
	Version: 0.2
	Created: 21/07/2013
	Author : Thomas Odulate
	NOTES  : Logging class that assists in logging to a datafile on an SD card	

******************************************************************************/
#ifndef _SDLOGGER_h
#define _SDLOGGER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <SD.h>
#include "RTC_DS3231.h"
#include "Debug.h"

enum LogType {WARN, INFO, DATA, OUTP };

class SDLogger
{
 private:
	 short _csPin;
	 bool _IsReady;
	 bool IsReady();
	 String _TargetFile;
	 bool SDCardInserted();
	 bool Log( String );
	 bool WriteToFile( String );

 public:
	LogType _logtype;
	SDLogger( short , const char*  );	
	bool Log( String, LogType );
	String GetTimeStamp( void );
	bool ClearLog();
};

#endif

