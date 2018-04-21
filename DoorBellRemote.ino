//Includes//

#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69, radio library.
#include <SPI.h>  //For comms with radio.
#include <LowPower.h> //get library from: https://github.com/lowpowerlab/lowpower, for sleeping micro.
#include <RFM69_ATC.h>//get it here: https://www.github.com/lowpowerlab/, radio library with power level control.
//#include <LDR10k.h>  //10k nominal resistance photoresistor library.
//#include <Thermistor10k.h>  //10k nominal resistance photoresistorl ibrary.

//Radio Settings//

#define NETWORKID     100  //the same on all nodes that talk to each other
#define NODEID        6  
#define GATEWAYID     1
#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "SaffellBot123456" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW    //uncomment only for RFM69HW! Remove/comment if you have RFM69W!

//Connections//

const int LEDPIN=9;
const int DOORBELLPIN=4;
const int REMOTEDOORBELL=5;
const int REMOTEALARM=6;


//Transmitting Variables//

char transmitBuffer[15];  //Buffer to send messages to base station.
int sendSize=0;  //Used for radio transmissions.

const char SENSORNAME4[3]="TL";
int sensorValue4=0;

const char SENSORNAME5[3]="SS";
int sensorValue5=0;

const char SENSORNAME6[3]="RT";
int sensorValue6=0;

//const char SENSORNAME2[3]={"TE"};
//const byte PRECISION2 = 2;  //Used for dtostrf() later.
//char floatBuffer2[PRECISION2+4];

//const char SENSORNAME3[3]={"LI"};
//const byte PRECISION3 = 2;  //Used for dtostrf() later.
//char floatBuffer3[PRECISION3+4];

//Light Variables//

//const int LDRPIN=A2;
//const int LDRRESISTOR=10000;
//float ldrRaw=0.0;
//float ldrFiltered=0.0;
//float ldrFilterFactor=0.9;

//Temperature Variables//

//const int THERMISTORPIN=A3;
//const int THERMISTORRESISTOR=10000;
//float thermistorRaw=0.0;
//float thermistorFiltered=0.0;
//float thermistorFilterFactor=0.7;

//Timing Control//

int environmentalCounter=0;
unsigned long environmentalTimer=0;
unsigned long doorbellTimer=0;
unsigned long doorbellDelay=0;
bool doorbellSend=0;

//Objects//

//LDR10k ldr(LDRPIN, LDRRESISTOR);
//thermistor10k thermistor(THERMISTORPIN, THERMISTORRESISTOR);
RFM69_ATC radio;

//Prototype Functions//

void ringDoorbell();
void environmental();
//void readLight();
//void readTemperature();

void setup()
{
	pinMode(LEDPIN, OUTPUT);  //Prevents LED from lighting.
	pinMode(DOORBELLPIN, INPUT);  //Pin that the doorbell trigger is attached to.
	pinMode(REMOTEDOORBELL, OUTPUT);  //Pin that triggers the doorbell.
	pinMode(REMOTEALARM, OUTPUT); //Pin that triggers the alarm.
	
	//digitalWrite(LEDPIN, HIGH);
	
	radio.initialize(FREQUENCY,NODEID,NETWORKID); //Startup radio
  	radio.encrypt(ENCRYPTKEY);
  	radio.enableAutoPower(-60);
  	
  	//ldrFiltered=ldr.readLDR();
  	//thermistorFiltered=thermistor.readThermistor();
	
	//Serial.begin(19200);
}

void loop()
{
	
	if ((millis()-doorbellTimer)>2000)
	{
		if (digitalRead(DOORBELLPIN)==0)
		{
			doorbellTimer=millis();
			doorbellSend=1;
		}
	}
	
	if (doorbellSend==1 && ((millis()-doorbellTimer)>5000))
	{
		//Serial.println("DOORBELL");
		sprintf(transmitBuffer, "$%d@DB#DB*",NODEID);
  		sendSize = strlen(transmitBuffer);
  		radio.sendWithRetry(GATEWAYID, transmitBuffer, sendSize);	
  		doorbellSend=0;
	}
	
	if (radio.receiveDone())
    {
    	if (radio.DATA[0]=='R' && radio.DATA[1]=='B')
    	{
    		ringDoorbell();
    	}
    	
    	else if (radio.DATA[0]=='A' && radio.DATA[1]=='L')
    	{
    		ringAlarm();
    	}
    }
    
   
   	if ((millis()-environmentalTimer)>60000)
   	{
    	environmentalTimer=millis();
    	environmental();
   	}
   
}

void ringDoorbell()
{
	digitalWrite(REMOTEDOORBELL, HIGH);
	delay(100);
	digitalWrite(REMOTEDOORBELL, LOW);
	
	return;
}	

void ringAlarm()
{
	digitalWrite(REMOTEALARM, HIGH);
	delay(100);
	digitalWrite(REMOTEALARM, LOW);
	
	return;
}
	
	
void environmental()
{
	//if (environmentalCounter==0 || environmentalCounter==5)
	//{
	//	readTemperature();
	//}
	
	//if (environmentalCounter==1 || environmentalCounter==6)
	//{
	//	readLight();
	//}
	
	if (environmentalCounter==2)
	{
		transmitLevel();
	}
	
	if (environmentalCounter==3)
	{
		signalStrength();
	}
	
	
	if (environmentalCounter==4)
	{
		enclosureTemperature();
	}
	
	
	if (environmentalCounter==9)
	{
		environmentalCounter=0;
	}
	
	environmentalCounter++;

	return;	
}

/*

void readLight()
{
	float ldrRaw=ldr.readLDR();
  	ldrFiltered=(ldrFilterFactor*ldrRaw)+((1-ldrFilterFactor)*ldrFiltered);
   	dtostrf(ldrFiltered, PRECISION2+3, PRECISION2, floatBuffer2);  //Converts battery voltage to a string to be sent out for debugging.
  	sprintf(transmitBuffer, "$%d@%s#%s*",NODEID,SENSORNAME2,floatBuffer2);
  	sendSize = strlen(transmitBuffer);
  	radio.sendWithRetry(GATEWAYID, transmitBuffer, sendSize);
  	
	return;
}

void readTemperature()
{
	float thermistorRaw=thermistor.readThermistor();
  	thermistorFiltered=(thermistorFilterFactor*thermistorRaw)+((1-thermistorFilterFactor)*thermistorFiltered);
    dtostrf(thermistorFiltered, PRECISION3+3, PRECISION3, floatBuffer3);  //Converts battery voltage to a string to be sent out for debugging.
  	sprintf(transmitBuffer, "$%d@%s#%s*",NODEID,SENSORNAME3,floatBuffer3);
  	sendSize = strlen(transmitBuffer);
  	radio.sendWithRetry(GATEWAYID, transmitBuffer, sendSize);
  	
  	return;
}
*/

void transmitLevel()
{
	sensorValue4=radio._transmitLevel;
  	sprintf(transmitBuffer, "$%d@%s#%d*",NODEID,SENSORNAME4,sensorValue4);
  	sendSize = strlen(transmitBuffer);
  	radio.sendWithRetry(GATEWAYID, transmitBuffer, sendSize);
  	sensorValue5=radio.RSSI;
  	
  	return;
}

void signalStrength()
{
	sprintf(transmitBuffer, "$%d@%s#%d*",NODEID,SENSORNAME5,sensorValue5);
  	sendSize = strlen(transmitBuffer);
  	radio.sendWithRetry(GATEWAYID, transmitBuffer, sendSize);
  	
  	return;
}

void enclosureTemperature()
{
	sensorValue6=radio.readTemperature(0)*1.8+32;
  	sprintf(transmitBuffer, "$%d@%s#%d*",NODEID,SENSORNAME6,sensorValue6);
  	sendSize = strlen(transmitBuffer);
  	radio.sendWithRetry(GATEWAYID, transmitBuffer, sendSize);
  	
  	return;
}