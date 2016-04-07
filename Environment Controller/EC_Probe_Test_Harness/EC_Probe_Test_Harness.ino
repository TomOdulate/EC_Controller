/****************************************************************************** 
	NAME   : EC_Probe_Test_Harness.ino
	Version: 0.1
	Created: 28/05/2012
	Author : Thomas Odulate
	NOTES  : Firmware used to test EC_Probe circuits.
	
	

******************************************************************************/
#include "EC_Probe_Types.h"
#include "Arduino.h"
#include "SimpleModbusMaster.h"

// led to indicate that a communication error is present
#define connection_error_led 13

//////////////////// Port information ///////////////////
#define baud 4800
#define timeout 1000
#define polling 500 // the scan rate

// If the packets internal retry register matches
// the set retry count then communication is stopped
// on that packet. To re-enable the packet you must
// set the "connection" variable to true.
#define retry_count 10 

// used to toggle the receive/transmit pin on the driver
#define TxEnablePin 2 

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum
{
  THL_PROBE_PACKET,
  //PACKET2,
  // leave this last entry
  TOTAL_NO_OF_PACKETS
};

// Create an array of Packets for modbus_update()
Packet packets[TOTAL_NO_OF_PACKETS];

// Create a packetPointer to access each packet
// individually. This is not required you can access
// the array explicitly. E.g. packets[PACKET1].id = 2;
// This does become tedious though...
packetPointer thl_Probe_packet = &packets[THL_PROBE_PACKET];
//packetPointer packet2 = &packets[PACKET2];

// The data from the PLC will be stored
// in the regs array
unsigned int thl_Probe_Regs[6];

#define GetFractionalPart(value) ((int)((value - (int)value)*100))
#define ToFractionalPart(value)  ((float)value / 100.00f )


void setup()
{
  // Read 6 registers starting at address 0, from thl_probe (network address 1).
  thl_Probe_packet->id = 1;
  thl_Probe_packet->function = READ_HOLDING_REGISTERS;
  thl_Probe_packet->address = 0;
  thl_Probe_packet->no_of_registers = 6;
  thl_Probe_packet->register_array = thl_Probe_Regs;
  
  // write the 9 registers to the PLC starting at address 3
  //packet2->id = 1;
  //packet2->function = PRESET_MULTIPLE_REGISTERS;
  //packet2->address = 0;
  //packet2->no_of_registers = 9;
  //packet2->register_array = regs;
	// P.S. the register_array entries above can be different arrays
  
	// Initialize modbus network settings etc...
	modbus_configure(baud, timeout, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS);
  
	pinMode(connection_error_led, OUTPUT);
}



void loop()
{
	unsigned int connection_status = modbus_update(packets);
	delay(200);
  
	if (connection_status != TOTAL_NO_OF_PACKETS)
	{
		digitalWrite(connection_error_led, HIGH);
		Serial.println("Error!");
		// You could re-enable the connection by:
		packets[connection_status].connection = true;
		delay(20);
		
	}
	else
	{
		digitalWrite(connection_error_led, LOW);
  
		// Output the value of the registers (same serial port!) so we can see them in the serial monitor.
		for(int i=0;i<6;i++) {
			Serial.print(thl_Probe_packet->register_array[i]);delay(1);Serial.print(":");delay(1); 
			if(i>=5)
			{
				Serial.println();
				delay(1);
				
			}
		}

		/*for(int i=0;i<6;i++) {
			packet1->register_array[i] = 0;
		}*/
		
		//delay(500);
		//for(int i=0;i<8;i++) {
		//	packet1->register_array[i]=0;		
		//}
		//delay(200);
	}
	   delay(300);// TODO <---  Do NOT loop too fast!  Check the polling var above! try to balance them!
}