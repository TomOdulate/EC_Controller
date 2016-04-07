#include "SDWebServer.h"
#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>

EthernetServer server(80);
SDWebServer webServer;

enum _settingsEnum  { MxTMP, MnTMP,Fan1Max,Fan1Min,Fan2Max,Fan2Min,HumMax,HumMin, Hys, Logging, NegFback, MaxNumSettings };
// String array used to detect post variables. Any post variables defined in .htm files must have a matching 
// definition here.
String _settingStrings[MaxNumSettings] = {"MxTMP\0","MnTMP\0","Fan1Max\0","Fan1Min\0","Fan2Max\0","Fan2Min\0","HumMax\0","HumMin\0","Hys\0","Log\0","NegFback\0"};

char logFilename[]	= "/ec.log";		// The name of then logfile.
int AmountToRead	= 2000;				// The number of bytes to read from the log file.

/*
	Constructor / initialiser. Must be called before using this class.
*/
void SDWebServer::init(uint8_t firstOctet,uint8_t secondOctet,uint8_t thirdOctet,uint8_t fourthOctet)
{
	byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
	IPAddress ip(firstOctet,secondOctet,thirdOctet,fourthOctet);
	
	// start the Ethernet connection and the server:
	Ethernet.begin(mac, ip);
	server.begin();
	
	#if defined(DEBUG_WEBSERVER)
		Serial.print("Web server is at ");
		Serial.println(Ethernet.localIP());
	#endif
}
/*
	Call this function any chance you get to check for ethernet data.  if there's any requests, 
	the function decides if its a GET or a POST, then parses the request.

	NB.  Note that the clients request string is read into 	memory in one go 
	(String s = client.readString();).  This is certianly not the best way to handle this, 
	since we have no control on what the client sent.  Could cause a buffer overflow.  
	Time permitting, I'd change this to read in a sent number of bytes at a time. 
*/
void SDWebServer::ProcessWebServer( ec_Data* data )
{	
	server.begin();
	EthernetClient client = server.available();
	this->DataChanged = false;

	if (client) 
	{
		#if defined(DEBUG_WEBSERVER)
			Serial.print("Web request: ");
		#endif

		while (client.connected())
		{
			if (client.available())
			{
				String s = client.readString();
				if( s.indexOf("GET") >-1 )
					ParseGETRequest(s, &client, data);
				else 
					if( s.indexOf("POST") >-1 )
						ParsePOSTRequest(s, &client, data);
					else 
					{
						client.flush();
						client.stop();
					}
			}
		}
	}
}
/*
	Parses the requested filename.  If its a special filename i.e "/feed" a page 
	is generated and returned otherwise the response is generated as normal.
*/
void SDWebServer::ParseGETRequest(String s, EthernetClient *client, ec_Data* data)
{
	// Parse the filename
	String rFile = s.substring(4, s.indexOf(' ',4) ) + "\0";
	rFile.toLowerCase();

	#if defined(DEBUG_WEBSERVER)
		Serial.println(rFile);
	#endif

	if(rFile.compareTo("/feed\0")==0)
		this->ProcessFeed( client,  data );
	else
	{	
		if(rFile.compareTo("/\0")==0)
			rFile = "/index.htm\0";

		char filename[20];
		rFile.toCharArray(filename,20);	
		ProcessResponse(filename, client, data);
	}
}
/*
	Generates a 'quick' string of text that contains the current sensor values.  
	The point of this function is to enable the server to provide a fast data 
	feed for a desktop or other app.

	A better way of doing this would probably be a UDP packet sent to a perticular 
	IP, since this avoids connection overheads and would be faster, however this 
	means managing the ip address' to send to.
*/
void SDWebServer::ProcessFeed(EthernetClient *client, ec_Data *data)
{
	char ascii[23];
	sprintf( ascii, "%02d.%02d,%02d.%02d,%03d\0", (int)data->TempNow, GetFractionalPart(data->TempNow),(int)data->HumNow,GetFractionalPart(data->HumNow), (int)data->LigNow );	
	client->println( ascii );
	client->stop();

#if defined(DEBUG_WEBSERVER)
	Serial.println("Processed feed\0");
#endif

}
/*
	Retuns the tail end of the log file to the web client.  The amount of data 
	read is set by the 'AmountToRead' variable.
*/
void SDWebServer::ProcessLog(EthernetClient *client, ec_Data *data)
{
	// Open the file 
	File f = SD.open(logFilename,FILE_READ);
	
	// If the file is large, skip to the last 'AmountToRead' bytes
	if( f.size() > AmountToRead )
	{
		// Move to the end of the file.
		f.seek( f.size() - AmountToRead );		

		// Now move forward until we find a carriage return, EOL
		bool flag = false;
		while(!flag)
		{
			if( f.peek() != 10)
				f.read();
			else
			{
				f.read();
				flag = true;
			}
		}
	}

	client->println("<pre>");

	while(f.available())		
		client->print((char)f.read());
	
	client->println("</pre>");
}
/*
	Handles reading post variables and storing them in our data object.
*/
bool SDWebServer::ParsePOSTRequest(String s, EthernetClient* client, ec_Data* data)
{	
	int Values[MaxNumSettings];	// Temporary storage for post variables.
	bool foundLogingVar = false;

	// Parse the filename
	String rFile = s.substring(5, s.indexOf(' ',5) ) + "\0";
	rFile.toLowerCase();

	if(rFile.compareTo("/\0")==0)
		rFile = "/index.htm\0";

	#if defined(DEBUG_WEBSERVER)
		Serial.println(rFile);
	#endif
	
	// Parse the query string
	int lastLine = s.lastIndexOf("\n")+1;
	String query = s.substring(lastLine);
	
	#if defined(DEBUG_WEBSERVER)
		Serial.print("\nQuery string is "); Serial.println(query);
	#endif

	// Loop through all accepted keys & assign store its value.
	for( int i = 0; i<MaxNumSettings; i++ )
	{
		// Find a key
		#if defined(DEBUG_WEBSERVER)
			Serial.print("Searching for "); Serial.println(_settingStrings[i]);
		#endif

		int StartOfKey = query.indexOf(_settingStrings[i]);
		int EndOfKey = query.indexOf("&",StartOfKey);
		if(EndOfKey<0)
			EndOfKey = query.length();

		// If a valid key was found, process its value
		if ( StartOfKey >= 0 )
		{			
			// Grab the value of this key, NB. int conversion and no floats!
			Values[i] = query.substring(StartOfKey+_settingStrings[i].length()+1,EndOfKey).toInt();
						
			#if defined(DEBUG_WEBSERVER)
				Serial.print("Found "); Serial.print(_settingStrings[i]); Serial.print(":"); Serial.println(Values[i]);
			#endif

			// Store the value. NB. There's no sanity check or validation here
			// Do this in the javascript for the page for speeds sake ;).
			switch(i)
			{
				case MxTMP:		data->TempMax		= Values[i]; break;
				case MnTMP:		data->TempMin		= Values[i]; break;
				case Fan1Max:	data->Fan1MaxSpeed	= Values[i]; break;
				case Fan1Min:	data->Fan1MinSpeed	= Values[i]; break;
				case Fan2Max:	data->Fan2MaxSpeed	= Values[i]; break;
				case Fan2Min:	data->Fan2MinSpeed	= Values[i]; break;
				case HumMax:	data->HumMax		= Values[i]; break;
				case HumMin:	data->HumMin		= Values[i]; break;
				case Hys:		data->Hysteresis	= Values[i]; break;
				case Logging:	data->Logging		= 1; foundLogingVar = true; break;
				case NegFback:	data->NegFeedBack	= Values[i]; break;
			}
		}
	}

	// Note that Logging is a checkbox, so if not set it won't be posted.
	data->Logging = foundLogingVar;

	// Flag that we have changes to the data object
	this->DataChanged = true;
	delay(10);

	// Reload the calling page.
	char filename[20];
	rFile.toCharArray(filename,20);
	client->flush();
	ProcessResponse(filename, client, data);
}
/*
	Sends a file to the requesting client, injecting Javascript into the response.
*/
void SDWebServer::ProcessResponse(char * filename, EthernetClient *client, ec_Data* data)
{
	if(!SD.exists(filename))
	 {
		client->println("HTTP/1.1 404 OK");
		client->println("Content-Type: text/html");
		client->println("Connection: close");  // the connection will be closed after completion of the response
		client->println();
		client->print( "404 Error" );
		#if defined(DEBUG_WEBSERVER)
			Serial.print("File not found : ");
		#endif
	 }
	 else
	 {
		client->println("HTTP/1.1 200 OK");
		client->println("Content-Type: text/html");
		client->println("Connection: close");	// the connection will be closed after completion of the response
		client->println();

		// Inject data values
		this->InjectDataValues( client, data, filename );

		if( strncmp("/ec.log", filename,9) == 0 )
			ProcessLog( client, data );
		else
		{
			// Load the file
			File f = SD.open(filename,FILE_READ);
			while( f.available() )		
				client->print((char)f.read());
		}
	 }
	
	client->clearWriteError();
	client->flush();
	client->stop();	
}
/*
	Sends data values to the requesting client as Javascript. Note that this function
	only inserts values into either the home.htm and settings.htm pages.
*/
void SDWebServer::InjectDataValues( EthernetClient *client, ec_Data* data, char* filename )
{
	char injectCode[150];	

	// Current sensor values
	if( strcmp(filename, "/home.htm\0")==0)
	{	
		sprintf(injectCode, 
			"<script type=\"text/javascript\">var tmp_now = %02d.%02d; var hum_now = %02d.%02d; var lig_now = %03d;</script>\0", 
			(int)data->TempNow,
			GetFractionalPart(data->TempNow),
			(int)data->HumNow,
			GetFractionalPart(data->HumNow),
			data->LigNow);
	
		client->println( injectCode );
		return;
	}

	// Current settings values
	if( strcmp(filename, "/settings.htm\0") == 0 ) 
	{
		client->println("<script type=\"text/javascript\">");
		
		sprintf(injectCode, 
			"var tmp_max = %02d.%02d; var tmp_min = %02d.%02d; var hum_max = %02d.%02d; var hum_min = %02d.%02d;\0", 
			(int)data->TempMax,
			GetFractionalPart(data->TempMax),
			(int)data->TempMin,
			GetFractionalPart(data->TempMin),
			(int)data->HumMax,
			GetFractionalPart(data->HumMax),
			(int)data->HumMin,
			GetFractionalPart(data->HumMin));
	
		client->println( injectCode );

		// Fan speeds settings.
		sprintf(injectCode, "var fan1_max = %d; var fan1_min = %d; var fan2_max = %d; var fan2_min = %d;\0", 
								data->Fan1MaxSpeed,
								data->Fan1MinSpeed,
								data->Fan2MaxSpeed,
								data->Fan2MinSpeed);
	
		client->println( injectCode );

		// other settings.
		sprintf(injectCode, "var hys = %d; var log = %d; var negFeed = %d;\0", data->Hysteresis, (data->Logging)?1:0,data->NegFeedBack);
		client->println( injectCode );
		client->println( "</script>\0" );
		return;
	}
}