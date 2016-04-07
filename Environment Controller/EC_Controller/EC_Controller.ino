/****************************************************************************** 
	NAME   : EC_Controller.ino
	Version: 0.1
	Created: 27/06/2013
	Author : Thomas Odulate
	NOTES  : Firmware for the Environmental Controller. 	

******************************************************************************/

// Arduino standard libraries
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <avr/eeprom.h>
#include "SimpleModbusMaster.h"

#include "Debug.h"
#include "EC_Interface.h"
#include "EC_Types.h"
#include "SDLogger.h"
#include "OutputsManager.h"
#include "EventManager.h"
#include "EC_TemperatureControl.h"
#include "SDWebServer.h"

EventManager EventMgr;								// Handles all 'event' timing stuff & provides a RTC for the system.
EC_Interface  ec_interface(SPLASH_SCREEN_DELAY);	// Handles user interface (LCD, rotary encoder & button).
SDLogger Logger(SD_SELECT, *LOGFILE);				// Provides logging to an SD card.
OutputsManager OutputMgr(OUT1, OUT2, OUT3, OUT4,	
	OUT5, OUT6,PWM1,PWM2, &Logger);					// Provides temperature control & controls physical outputs.
ec_Data data;										// Storage struct for sharing application data. 
DateTime nextLogTime;								// Stores the timing of next loggging event.
unsigned long lastModbusScan = 0;					// Used to time calls to modbus network.
bool networkError = false;							// Flag indicates an modbus network error.

// test
void setup()
{
	// Attach interrupts for the rotary encoder.
	//PCintPort::attachInterrupt(1, doEncoderA,HIGH);
	//PCintPort::attachInterrupt(2, doEncoderB,HIGH);
	//attachInterrupt(1, doEncoderA,HIGH);
	//attachInterrupt(2, doEncoderB,HIGH);

	// Start the serial port for debug messages
	Serial.begin(57600);

	// Load settings from EEPROM
	LoadSettings();
	
	// Initialise the user interface
	ec_interface.SetupInterface();

	// Create packets for Modbus network.
	// Read 6 registers starting at address 0, from thl_probe (network address 1).
	thl_Probe_packet->id = 1;
	thl_Probe_packet->function = READ_HOLDING_REGISTERS;
	thl_Probe_packet->address = 0;
	thl_Probe_packet->no_of_registers = 6;
	thl_Probe_packet->register_array = thl_Probe_Regs;

	// Initialize modbus network settings and process it once now, so that we have something to 
	// display when the LCD interface starts.
	modbus_configure(baud, timeout, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS);	
	modbus_update(packets); 
   
	// Sets the RTC to the date & time this sketch was compiled, if its out of date	
	DateTime now = EventMgr.now();
    DateTime compiled = DateTime(__DATE__, __TIME__);
	if (now.unixtime() < compiled.unixtime()) {
		Serial.println("RTC is older than compile time! Updating");		
		EventMgr.SetDateTime(DateTime(__DATE__, __TIME__)); 
	}

	// Initialise the web server.
	webServer.init( 192,168,0,177 );

	// Draw the display
	ec_interface.DrawInterface();

	// Log that the system has started.
	Log("Sytem Initialised\0",INFO);
}

void loop()
{
	ec_interface.ProcessUserInterface();	// Check interface for user input.

	if( ec_interface.MODE == NORMAL )
	{		
		ProcessModbusNetwork();				// Check the network
		ProcessPWMOutputs();				// Set Fan speeds
		ProcessEvents(false);				// Check for timed events
		ProcessDisplay();					// Update the display
		ProcessWebServer();					// Check for web requests
		ProcessDataLogging();				// Log data
	}
}


void ProcessPWMOutputs()
{	
	OutputMgr.MaintainTemperature( data.TempNow, data.TempMin, data.TempMax, data.Hysteresis, data.NegFeedBack,
							data.Fan1MinSpeed, data.Fan1MaxSpeed, data.Fan2MinSpeed, 
							data.Fan2MaxSpeed );
}

void ProcessEvents(bool FirstTime)
{
	bool newState;
	byte outputNumber;
	char logStr[21];	

	if(FirstTime) // If first time, just restore the relay states.
	{
		
	}
	else  // Loop through the list of events &  if an event is due, Process it.
	{		
		// Get the current time
		uint32_t now = EventMgr.now().unixtime();
		for(int eventID=0;eventID<100;eventID++)
		{
			// If the event is due...
			if((EventMgr.EventData[eventID].EventTime <= now) & (EventMgr.EventData[eventID].Type == USER))
			{				
				OutputMgr.SetState(EventMgr.EventData[eventID].Output-1, EventMgr.EventData[eventID].newState );
				sprintf(logStr,"Output%d %s (%d)",EventMgr.EventData[eventID].Output, (EventMgr.EventData[eventID].newState == 0)?"off":"on",OutputMgr.GetState());					
				Log(logStr, OUTP);				

				// If its a one-off, remove it.
				if(EventMgr.EventData[eventID].Repeat == ONESHOT)
				{
					EventMgr.RemoveEvent(eventID);
					continue;
				}
				else
				{
					EventMgr.EventData[eventID].EventTime = now + (EventMgr.EventData[eventID].Interval * EventMgr.EventData[eventID].Repeat);
				}
			}
		}
	}
}

void ProcessModbusNetwork()
{
	if((millis() - lastModbusScan) > ProcessModbusNetworkInterval )
	{
		lastModbusScan = millis();
		unsigned int connection_status = modbus_update(packets);	
	
		if (connection_status != TOTAL_NO_OF_PACKETS)
		{
			//networkError = true;
			packets[connection_status].connection = true;		// Re-enable the connection after error
		}
		else
		{
			// Store the current sensor values 
			data.TempNow = thl_Probe_packet->register_array[0] + (thl_Probe_packet->register_array[1]/100.0f);
			data.HumNow	= thl_Probe_packet->register_array[2] + (thl_Probe_packet->register_array[3]/100.0f);
			data.LigNow	= thl_Probe_packet->register_array[4];
			ChkRecords(data);
		}
	}
}

void ProcessDataLogging()
{    
	if(data.Logging)
	{
		DateTime now = EventMgr.now();
		if( nextLogTime.unixtime() > now.unixtime())
			return;
	
		nextLogTime =  (now.unixtime() + (LOG_INTERVAL));  // set the next time to log.
	
		char ascii[41];
		
		sprintf( ascii, "%02d.%02d,%02d.%02d,%03d", 
				(int)data.TempNow, 
				GetFractionalPart(data.TempNow),
				(int)data.HumNow,
				GetFractionalPart(data.HumNow), 
				data.LigNow
				);	
		
		Log(ascii, DATA);
	}
}

void ProcessWebServer()
{
	webServer.ProcessWebServer(&data);
	if(webServer.DataChanged)
	{
		SaveSettings();
	}
}

void ProcessDisplay()
{
	ec_interface.UpdateHomeScreen(data.TempNow, data.TempHistoricalMin, data.TempHistoricalMax, 
									data.HumNow, data.HumHistoricalMin, data.HumHistoricalMax, 
									data.LigNow, networkError);
}


// Helper functions.
static void Log(char* ascii, LogType type)
{	
	char logString[46];
	sprintf(logString, "%s,%s", GetTimeStamp(), ascii);	
	Logger.Log( logString, type );
}

static char* GetTimeStamp()
{
	const int len = 20;
	static char buf[len];
	DateTime now = EventMgr.now();
	return now.toString(buf,len);
}

static void ChkRecords(ec_Data &data)
{
	if(data.TempNow > data.TempHistoricalMax)
		data.TempHistoricalMax = data.TempNow;

	if(data.HumNow > data.HumHistoricalMax)
		data.HumHistoricalMax = data.HumNow;	
	
	if(data.TempNow < data.TempHistoricalMin)
		data.TempHistoricalMin = data.TempNow;
	else
		if (data.TempHistoricalMin == NULL)
			data.TempHistoricalMin = data.TempNow;
	
	if(data.HumNow < data.HumHistoricalMin)
		data.HumHistoricalMin = data.HumNow;
	else
		if (data.HumHistoricalMin == NULL)
			data.HumHistoricalMin = data.HumNow;
}

void doEncoderA()
{
	if(ec_interface.MODE != NORMAL)
	{	
		//detachInterrupt(0);
		ec_interface.doEncoderA();
		//attachInterrupt(0, doEncoderA,RISING);
	}
	Serial.println("Encoder A triggered"); 
}

void doEncoderB()
{
	if(ec_interface.MODE != NORMAL)
	{
		//detachInterrupt(1);
		ec_interface.doEncoderB();
		//attachInterrupt(1, doEncoderB,HIGH);
	}	
	Serial.println("Encoder B triggered");
}

uint16_t CalculateSettingsCRC() 
{
	uint16_t crc = 0;
	byte *p = (byte*)&data;
	
	for( byte i=0; i<sizeof(ec_Data); i++ )
		crc+=p[i];
	
	return crc;
}

bool CheckSettingsCRC( uint16_t crc ) 
{
	bool a = false;
	(crc == CalculateSettingsCRC())?a = true:a = false;
	return a;
}

static void LoadSettings()
{	
	uint16_t crc, crcCheck;
	byte *p;
	eeprom_read_block((void*)&data, (void*)0, sizeof(ec_Data));		
	crc = eeprom_read_word((uint16_t*)sizeof(ec_Data));

	if( !CheckSettingsCRC(crc) )	
	{		
		#if defined(DEBUG_MAIN)
			Serial.println("Unable to load settings from EEPROM");
		#endif
		LoadDefaultSettings();
		SaveSettings();
	}
	else
	{	
		#if defined(DEBUG_MAIN)
			Serial.println("Loading settings from EEPROM");
		#endif
	}
}

void LoadDefaultSettings()
{	
	#if defined(DEBUG_MAIN)
		Serial.println("Loading default values.");
	#endif
	
	data.Fan1MaxSpeed	= 100;
	data.Fan1MinSpeed	= 0;
	data.Fan2MaxSpeed	= 100;
	data.Fan2MinSpeed	= 0;
	data.TempMax		= 30;
	data.TempMin		= 19;
	data.HumMax			= 60;
	data.HumMin			= 30;
	data.Logging		= true;
	data.NegFeedBack	= 25;
}

static void SaveSettings()
{
	uint16_t crc = CalculateSettingsCRC();	
	eeprom_write_block((void*)&data, (void*)0, sizeof(ec_Data));	
	eeprom_write_word((uint16_t*)sizeof(ec_Data),crc);	
	
	#if defined(DEBUG_MAIN)
		Serial.println("Saved settings to EEPROM");
	#endif
}
