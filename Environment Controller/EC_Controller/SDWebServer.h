// SDWebServer.h

#ifndef _SDWEBSERVER_h
#define _SDWEBSERVER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#define DEBUG
#define SD_SELECT 4

#include "Debug.h"
#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>

typedef struct ec_Data
{
	// Current Values
	float TempNow;
	float HumNow;
	int LigNow;

	float TempHistoricalMax;
	float TempHistoricalMin;
	float HumHistoricalMax;
	float HumHistoricalMin;
  
	// User settings
	short Fan1MaxSpeed;
	short Fan1MinSpeed;
	short Fan2MaxSpeed;
	short Fan2MinSpeed;
	short TempMax;
	short TempMin;
	short HumMax;
	short HumMin;
	bool  Logging;
  
	// System settings
	short Hysteresis;
	short NegFeedBack;
};

#define GetFractionalPart(value) ((int)((value - (int)value)*100))

static String GetRAsString(float value)
{
	int major = (int)value;
	int fraction = GetFractionalPart(value);

	String s = "";
	if(major < 10) s.concat("0");	
	s.concat(major);
	s.concat('.');	
	if(fraction <10) s.concat("0");
	s.concat(fraction);
	return s;
};

class SDWebServer
{
 private:
	void InjectDataValues(EthernetClient*, ec_Data*, char*);
	void ProcessFeed(EthernetClient *client, ec_Data*);
	void ProcessLog(EthernetClient*, ec_Data*);
	void ParseGETRequest(String s, EthernetClient *client, ec_Data*);
	bool ParsePOSTRequest(String s, EthernetClient *client, ec_Data*);
	void ProcessResponse(char * filename, EthernetClient *client, ec_Data*);
	String LoadWebPage(char *Filename, ec_Data*);
	String ParseTokens(String txt, ec_Data*);
 public:
	void ProcessWebServer( ec_Data* );
	void init(uint8_t firstOctet,uint8_t secondOctet,uint8_t thirdOctet,uint8_t fourthOctet);
	bool DataChanged;
};

extern SDWebServer webServer;

#endif