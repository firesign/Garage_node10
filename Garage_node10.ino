 /* 
 Jeenode Garage
 
 Uses two DHT22 temp & humidity sensors on Node 10.
 
 Code from Radioblip
 2012-05-09 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
 
 For monitoring Temperature, Humidity and Carbon Monoxide in the garage and wormbox.
 Also checks if the garage door is open.
 
 The CO sensor heater must be cycled at 1.4v for 90 seconds and 5v for 60 seconds.
 At the time just before the 1.4v to 5v transition, the sensor can be read.
 The other sensors are read at this time as well, and the data is then sent by radio.
 
 */

#include <JeeLib.h>
#include <avr/sleep.h>
#include "DHT.h"

#define DHTPIN 8	// Garage Temp/Humidity
#define DHTPIN2 9	// Wormbox Temp/Humidity
#define DHTTYPE DHT22   // DHT 22  (AM2302)



#define BLIP_NODE 10    // wireless node ID to use for sending blips
#define BLIP_GRP  212   // wireless net group to use for sending blips
//#define BLIP_ID   1   // set this to a unique ID to disambiguate multiple nodes
#define SEND_MODE 2     // set to 3 if fuses are e=06/h=DE/l=CE, else set to 2

DHT dht(DHTPIN, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
int COPIN = 3;		// Carbon Monoxide sensor located at A3
int coValue = 0;

int RELAYPIN = 7;	// Relay located at DIO7
boolean FLAG = 0;	// If 0, enter 90s 1.4v, if 1, enter 60s 5v

// Garage Door Setup
int garageDoorPin = 4;      // Garage Door open detect microswitch
//int ledPin = 5;             // Garage Door open LED

struct {
    int temp;			// garage temperature
    int hum;			// garage humidity
    int temp2;			// wormbox temperature
    int hum2;			// wormbox humidity
    int co;	        	// Carbon Monoxide level
    byte garageDoorOpen;	// Garage Door open indicator switch
    //int garageDoorOpen;
} payload;

// this must be defined since we're using the watchdog for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

void setup() {
    Serial.begin(9600);
    Serial.println("starting...");
    
    //pinMode(ledPin, OUTPUT);            // Garage Door indicator LED
    pinMode(garageDoorPin, INPUT);      // garage door
    digitalWrite(garageDoorPin, HIGH);  // enable pullup resistors
    
    dht.begin();			// Garage DHT22
    dht2.begin();			// Wormbox DHT22
    
    //analogReference(EXTERNAL);	// connect 3.4V to AREF (pin 21)
    pinMode (COPIN, INPUT);		// set up CO sensor input
    pinMode (RELAYPIN, OUTPUT);		//

    rf12_initialize(BLIP_NODE, RF12_433MHZ, BLIP_GRP);
    // see http://tools.jeelabs.org/rfm12b
    rf12_control(0xC040); 		// set low-battery level to 2.2V i.s.o. 3.1V
    rf12_sleep(RF12_SLEEP);
}

static byte sendPayload () {
    rf12_sleep(RF12_WAKEUP);
    while (!rf12_canSend())
    rf12_recvDone();
    rf12_sendStart(0, &payload, sizeof payload);
    rf12_sendWait(SEND_MODE);
    rf12_sleep(RF12_SLEEP);
  
}


void loop() {
    if (FLAG == 0) {
	Serial.println("Heater now at 1.4v");
	digitalWrite(RELAYPIN, FLAG);
	FLAG = 1;
	//Sleepy::loseSomeTime(9000); //wake up after 90 seconds
	delay(90000);
    } else {
	
	// CHECK SENSORS AND SEND DATA  ***********************************
	// CO Sensor
	coValue = 1000 - (analogRead(COPIN));
	payload.co = abs(1024 - coValue);	// CO levels are inverse to sensor value
	Serial.print("CO Level: ");
	Serial.println(payload.co);
	
	// Garage Door
	byte isTheDoorOpen = digitalRead(garageDoorPin);
	if (isTheDoorOpen == 0){		
	    //digitalWrite(ledPin, 1);		// garage door is open
	    payload.garageDoorOpen = 0;
	}
	else {
	    //digitalWrite(ledPin, 0);		// garage door is open
	    payload.garageDoorOpen = 1;
	}
	Serial.print("Garage Door: ");
	Serial.println(payload.garageDoorOpen);
	    
	// DHT SECTION
	int tt, hh, tt2, hh2;
	float h = dht.readHumidity();
	float h2 = dht2.readHumidity();
	delay(200);
	float t = dht.readTemperature();
	float t2 = dht2.readTemperature();
	if (isnan(t) || isnan(h) || isnan(t2) || isnan(h2)) {
	     Serial.println("Failed to read from a DHT sensor");
	} else {	
	     t = t * 10;     
	     tt = (int) t;
	     h = h * 10;
	     hh = (int) h;
	     t2 = t2 * 10;     
	     tt2 = (int) t2;
	     h2 = h2 * 10;
	     hh2 = (int) h2;
	     Serial.print("Garage humidity: "); 
	     Serial.print(hh);
	     Serial.print(" %\t");
	     Serial.print("Garage temperature: "); 
	     Serial.print(tt);
	     Serial.println(" *C");
	     Serial.print("Wormbox humidity: "); 
	     Serial.print(hh2);
	     Serial.print(" %\t");
	     Serial.print("Wormbox temperature: "); 
	     Serial.print(tt2);
	     Serial.println(" *C");
	     Serial.println(" ");
	}  
	payload.temp = tt;
	payload.hum = hh;
	payload.temp2 = tt2;
	payload.hum2 = hh2;
	sendPayload();
	
	Serial.println("Heater now at 5v");
	digitalWrite(RELAYPIN, FLAG);
	FLAG =0;
	
	delay(60000);
	//Sleepy::loseSomeTime(6000); //wake up after 60 seconds
    }
}
