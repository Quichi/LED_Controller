	#include "Arduino.h"
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "configuration.h"
#include "Ticker.h"
#include "WiFi_Credentials.h"
#include "config_adv.h"
#include <EEPROM.h>


uint8_t brightness[3];
uint8_t data[3];
uint8_t controlByte;

WiFiUDP Udp;
Ticker ticker;

char* ssid     = HOME_SSID;
char* pass = HOME_PASS;

int keyIndex = 0;            // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
int loop_time = 0;
uint8_t isLocal = 0;
configuration *config;

IPAddress broadcastIP(255,255,255,255);

void incrementMilli()
{
	loop_time++;
}

void setup()
{
	Serial.begin(BAUDRATE);

	for(int i=0; i<3; i++)
	{
		data[i]=0;
	}

	getDefaultValues();

	ticker.attach(.001, incrementMilli);

	if (NODE_ID != 0)
	{
		connectToWifi();
		Udp.begin(UDP_PORT);
	}

	pinMode(PWM_RED_PIN, OUTPUT);
	pinMode(PWM_GREEN_PIN, OUTPUT);
	pinMode(PWM_BLUE_PIN, OUTPUT);
}

void loop()
{
	int messageCount = 0;

	while(1)
	{
		if((loop_time >= 1000) && (config->status_messages==true))
		{
			loop_time = 0;
			messageCount++;
			Serial.println(messageCount);

			//statusMessageUDP();
		}
		getIncomingPackets();
		configure();

		setRGBChannels();
	}

}

void getIncomingPackets()
{
	int packetSize;
	IPAddress remoteIp;
	uint8_t len = 0;
	uint8_t intBuffer[255];

	packetSize = Udp.parsePacket();

	if (packetSize == 4) {

		/* Check for Multicast Message */
		if (Udp.destinationIP() != broadcastIP)
		{
			isLocal = 1;
		}
		else
		{
			isLocal = 0;
		}

		remoteIp = Udp.remoteIP();

	    /* read the packet into packetBufffer */
	    len = Udp.readBytes(intBuffer, 4);
	    if (len > 0)
	    {
	    	intBuffer[len] = 0;
	    }

	    controlByte = intBuffer[0];
	    data[0] = intBuffer[1];
	    data[1] = intBuffer[2];
	    data[2] = intBuffer[3];

	    Serial.print("Received packet of size ");
	    Serial.println(packetSize);
	    Serial.print("From ");
	    Serial.print(remoteIp);
	    Serial.print(", port ");
	    Serial.println(Udp.remotePort());
	    Serial.print("isLocal: ");
	    Serial.println(isLocal);
	    Serial.print("Data:  ");
	    Serial.print(intBuffer[0]);
	    Serial.print(intBuffer[1]);
	    Serial.print(intBuffer[2]);
	    Serial.println(intBuffer[3]);
	  }
}

void setRGBChannels()
{
	analogWrite(PWM_RED_PIN, PWM_SCALING_FACTOR * brightness[0]);
	analogWrite(PWM_GREEN_PIN, PWM_SCALING_FACTOR * brightness[1]);
	analogWrite(PWM_BLUE_PIN, PWM_SCALING_FACTOR * brightness[2]);
}

void configure()
{
	uint8_t i;

	switch(controlByte)
	{
	case SET_BRIGHTNESS_ID:
		if(isLocal)
		{
			for (i=0; i<3; i++)
			{
				brightness[i] = data[i];
			}
			Serial.println("Set Brightness");
		}
		break;

	case SEND_OWN_STATUS_ID:		/* STATUS REQUEST */
		statusMessageUDP();
		Serial.println("Send Status");
		break;

	case SWITCH_OFF_ID: 	/* OFF */
		Serial.print("Switch off: ");
		if (isLocal || data[0]==ROOM || data[0]==ALLROOMS)
		{
			for(i=0; i<3; i++)
			{
				brightness[i]=0;
			}
			Serial.println("successful");
		}
		else
		{
			Serial.println("failed");
		}
		break;

	case SWITCH_ON_ID: 	/* ON */
		Serial.print("Switch o: ");
		if(isLocal || data[0]==ROOM || data[0]==ALLROOMS)
		{
			for(i=0; i<sizeof(config->default_rgb); i++)
			{
				brightness[i] = config->default_rgb[i];
			}
			Serial.println("successful");
		}
		else
		{
			Serial.println("failed");
		}
		break;

	case SET_DEFAULTS_ID:
		if(isLocal)
		{
			Serial.println("Save Default Values");
			for (i=0; i<sizeof(config->default_rgb); i++)
			{
				config->default_rgb[i] = data[i];
			}
			setDefaultValues();
		}
		break;

	case REQUEST_DEFAULTS_ID:
		Serial.println("Sending Default Values");
		getDefaultValues();
		defaultsMessageUDP();
		break;

	case SET_STATUSMESSAGES_ID:
		Serial.println("Send Status Messages: "+config->status_messages);
		if((data[0]==0x01) && (data[1]==0x02) && (data[2]==0x03))
		{
			config->status_messages=true;
		}
		else
		{
			config->status_messages=false;
		}
		break;

	default:
		break;
	}
	controlByte = DO_NOTHING;

}

void connectToWifi()
{
	IPAddress deviceIP(192,168,0, 200+ROOM+NODE_ID);
	IPAddress gatewayIP(192,168,0,1);
	IPAddress subnetIP(255,255,255,0);

	delay(5000);

	/* Clear Wifi Settings in EEPROM */
	WiFi.disconnect();
	WiFi.softAPdisconnect();
	WiFi.mode(WIFI_OFF);
	delay(500);

	/* Reconfigure and Connect to Wifi */
	WiFi.mode(WIFI_STA);
	WiFi.config(deviceIP, gatewayIP, subnetIP);
	WiFi.begin(ssid, pass);

	Serial.print("Connecting to ");
	Serial.println(ssid);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}

	Serial.println("");

	Serial.print("Connected to ");
	Serial.println(WiFi.SSID());

	Serial.print("Local Device IP: ");
	Serial.println(WiFi.localIP());
}

/**
 * Broadcast a status Message with current brightness values
 */
void statusMessageUDP()
{	//TODO:
	uint8_t statusMessage[4];

	statusMessage[0] = STATUS_ID;
	for(int i=0; i<3; i++)
	{
		statusMessage[i+1] = brightness[i];
	}

	Udp.beginPacket(broadcastIP, Udp.remotePort());
	Udp.write(statusMessage,sizeof(statusMessage));
	Udp.endPacket();
}
void defaultsMessageUDP()
{
	uint8_t defaultsMessage[4];

	defaultsMessage[0] = RESPONSE_DEFAULTS_ID;
	for(uint8_t i=0; i<sizeof(config->default_rgb); i++)
	{
		defaultsMessage[i+1]= config->default_rgb[i];
	}

	Udp.beginPacket(broadcastIP, Udp.remotePort());
	Udp.write(defaultsMessage,sizeof(defaultsMessage));
	Udp.endPacket();
}

void setDefaultValues()
{
	EEPROM.begin(sizeof(configuration));

	for(uint8_t i=0; i<sizeof(config); i++)
	{
		EEPROM.write(CONFIGURATION_ADDR+i, *((char*)&config+i));
	}

	EEPROM.commit();
	EEPROM.end();
}

void getDefaultValues()
{
	EEPROM.begin(sizeof(configuration));

	for(uint8_t i=0; i<sizeof(config); i++)
	{
		*((char*)config+i) = EEPROM.read(CONFIGURATION_ADDR+i);
	}

	EEPROM.end();
}

