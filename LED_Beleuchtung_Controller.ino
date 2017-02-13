#include "Arduino.h"
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "configuration.h"
#include "Ticker.h"
#include "WiFi_Credentials.h"

char* ssid     = HOME_SSID;
char* pass = HOME_PASS;

int keyIndex = 0;            // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
int loop_time = 0;

uint8_t dim[3];
uint8_t controlByte;

WiFiUDP Udp;
IPAddress deviceIP, gatewayIP, subnetIP;
Ticker ticker;

void incrementMilli()
{
	loop_time++;
}

void setup()
{
	Serial.begin(9600);

	for(int i=0; i<3; i++)
	{
		dim[i]=0;
	}

	//ticker.attach(.001, incrementMilli);

	deviceIP  = IPAddress(192,168,0,200+NODE_ID);
	gatewayIP = IPAddress(192,168,0,1);
	subnetIP  = IPAddress(255,255,255,0);

	if (NODE_ID != 0)
	{
		//WiFi.config(deviceIP, gatewayIP, subnetIP);
		delay(5000);

		Serial.print("Connecting to ");
		Serial.println(ssid);

		WiFi.begin(ssid, pass);

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

		Udp.begin(UDP_PORT);
	}

	pinMode(PWM_RED_PIN, OUTPUT);
	pinMode(PWM_GREEN_PIN, OUTPUT);
	pinMode(PWM_BLUE_PIN, OUTPUT);
}

void loop()
{
	int iteration = 0;
	int messageCount = 0;

	while(1)
	{
		if ((iteration % 100) == 0)
		{
			messageCount++;
			Serial.println(messageCount);
		}

		evaluateIncomingPackets();
		setRGBChannels();


//		if(controlByte == 0)
//		{
//			setRGBChannels();
//		}
//		else
//		{
//			configure();
//		}
//
//		while(loop_time<.01)
//		{
//			;
//		}

		loop_time = 0;
		iteration++;
		delay(10);
	}

}

void evaluateIncomingPackets()
{
	int packetSize;
	IPAddress remoteIp;
	uint8_t len = 0;
	uint8_t intBuffer[255];

	packetSize = Udp.parsePacket();

	if (packetSize) {
		remoteIp = Udp.remoteIP();

	    Serial.print("Received packet of size ");
	    Serial.println(packetSize);
	    Serial.print("From ");
	    Serial.print(remoteIp);
	    Serial.print(", port ");
	    Serial.println(Udp.remotePort());

	    // read the packet into packetBufffer
	    len = Udp.readBytes(intBuffer, 255);
	    if (len > 0)
	    {
	    	intBuffer[len] = 0;
	    }
	    controlByte = intBuffer[0];
	    dim[0] = intBuffer[1];
	    dim[1] = intBuffer[2];
	    dim[2] = intBuffer[3];

	    Serial.print("Data:  ");
	    Serial.print(intBuffer[0]);
	    Serial.print(intBuffer[1]);
	    Serial.print(intBuffer[2]);
	    Serial.println(intBuffer[3]);

	    // send a reply, to the IP address and port that sent us the packet we received
//	    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
//	    Udp.write(ReplyBuffer);
//	    Udp.endPacket();
	  }
}

void setRGBChannels()
{
	analogWrite(PWM_RED_PIN,4* dim[0]); //TODO: values auf 4 Byte ändern
	analogWrite(PWM_GREEN_PIN,4* dim[1]);
	analogWrite(PWM_BLUE_PIN,4* dim[2]);
}

void configure()
{
	for(int i=0; i<3; i++)
	{
		dim[i] = 0;
	}

	//TODO:
	switch(controlByte)
	{
	case 1:
		break;

	default:
		break;
	}

}


void replyMessageUDP()
{	//TODO:

}

