#include "SDLogger.h"

SDLogger::SDLogger(short csPin, const char* logFile)
{
	this->_TargetFile = logFile;
	this->_csPin = csPin;
	pinMode(53, OUTPUT);	 // make sure that the default chip select pin is set to output
  
	// see if the card is present and can be initialized:
	if ( !IsReady() ) 
	{
		this->_IsReady = false;
		#if defined(DEBUG_LOGGING)
			Serial.println("SD Card is NOT ready");
		#endif
	} else {
		this->_IsReady = true;
		#if defined(DEBUG_LOGGING)
			Serial.println("SD Card is ready");
		#endif
	}
}

bool SDLogger::IsReady()
{
	return (SD.begin(this->_csPin)  & this->SDCardInserted());
}

bool SDLogger::Log(String s, LogType type)
{
	// Add log type to the string.
	String stmp;

	switch(type)
	{
		case INFO: stmp = "INFO,"; break;
		case WARN: stmp = "WARN,"; break;
		case OUTP: stmp = "OUTP,"; break;
		default:   stmp = "DATA,"; break;
	}

	return this->Log(stmp + s );
}

bool SDLogger::Log( String s )
{
	if( this->_IsReady )
		return this->WriteToFile( s );
	else
	{
		// Try to reinitialise the SD card.
		if( IsReady() )
		{
			this->_IsReady = true;
			return this->WriteToFile( s );
		}
	}
	
	#if defined(DEBUG_LOGGING)
		Serial.println("SD card not ready, unable to log");
	#endif

	return false;
}

bool SDLogger::WriteToFile( String s)
{
	bool success = false;

	char f[50];
	_TargetFile.toCharArray( f, s.length() );
		
	// open the file		
	File myFile = SD.open( f, FILE_WRITE );  
		
	// append txt
	char ascii[50];		
	s.toCharArray( ascii, 50);	
	myFile.println( ascii );
		
	// close file
	myFile.close();	

	success = true;

	#if defined(DEBUG_LOGGING)
		Serial.println(ascii);
	#endif

	return success;
}

bool SDLogger::SDCardInserted()
{
	// Check there is a card inserted, return false if not
	return true;
}

bool SDLogger::ClearLog()
{
	// Simply delete the logfile, it will be
	// recreated on next log attempt.

	char f[15];
	_TargetFile.toCharArray( f, _TargetFile.length() );
	return SD.remove(f);
}