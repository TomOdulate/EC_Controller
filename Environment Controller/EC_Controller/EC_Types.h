// EC_Types.h

#ifndef _EC_TYPES_h
#define _EC_TYPES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "SimpleModbusMaster.h"

#define DEBUG									// Enables debugging to serial port
#define FIRMWARE_VERSION	0.1					// Firmware version displayed to customer
#define SD_SELECT			4					// CS select pin for the SD card
#define LOG_INTERVAL		30					// The log interval in seconds
static const char* LOGFILE[9] = {"/EC.log\0"};	// The name and path of the logfile
#define SPLASH_SCREEN_DELAY	1000				// How long to show the splash screen in miliseconds

//Output  pin definitions.
#define OUT1 22
#define OUT2 23
#define OUT3 24
#define OUT4 25
#define OUT5 26
#define OUT6 27
#define PWM1 2
#define PWM2 3

// Helper macros
#define GetFractionalPart(value) ((int)((value - (int)value)*100))
#define ToFractionalPart(value)  ((float)value / 100.00f )

// SimpleModbus definitions and constants
#define baud	28800L							// Baud rate for modbus communication
#define timeout 100								// the modbus packet timeout
#define polling 200								// the modbus scan rate
#define ProcessModbusNetworkInterval 500L		// Controls how often a modbus update is performed in miliseconds
#define retry_count 2	// If the packets internal retry register = retry count, communication is stopped
						// on that packet. To re-enable the packet you must set the "connection" variable to true.
#define TxEnablePin 6	// used to toggle the receive/transmit pin on the driver

// This is the easiest way to create new packets Add as many as you want. TOTAL_NO_OF_PACKETS is 
// automatically updated.
enum ModbusPackets
{
  THL_PROBE_PACKET,								// Packet definition for an EC Temp / Humidity probe.
  //PACKET2,
  //PACKET3, etc, etc
  TOTAL_NO_OF_PACKETS							// NO NOT REMOVE this last entry!
};

// Create an array of Packets for modbus_update()
Packet packets[TOTAL_NO_OF_PACKETS];

// Create a packetPointer to access each packet individually. This is not required you can access
// the array explicitly. E.g. packets[PACKET1].id = 2. This does become tedious though...
packetPointer thl_Probe_packet = &packets[THL_PROBE_PACKET];

// The data from the PLC (Temperate / humidity / light probe) will be stored in the regs array
unsigned int thl_Probe_Regs[6];

static String GetReadingAsString(float value)
{
	String s = "";
	s.concat((int)value);
	s.concat('.');
	s.concat(GetFractionalPart(value));
	return s;
};


#endif