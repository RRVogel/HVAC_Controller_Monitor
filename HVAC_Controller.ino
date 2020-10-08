#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <cactus_io_AM2315.h>

#define CE_PIN  10
#define CSN_PIN 9

const byte thisSlaveAddress[5] = {'S','E','N','D','R'};

RF24 radio(CE_PIN, CSN_PIN);
AM2315 am2315;

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long senseIntervalMillis = 2000;

int controlData[2] = {1, 35};

int ackData[5];
bool newData = false;

int sensorPin = A0;
float sensorValue = 0;
float humidity = 0;

const int coolPin = 7;
const int heatPin = 6;
const int fanPin = 5;
const int humidPin = 3;
const int ductPin = 2;

unsigned int pressure;
uint8_t heatState;
uint8_t fanState;
uint8_t coolState;
uint8_t systemState = 2;
uint8_t humidState = LOW;
uint8_t ductState = HIGH;
unsigned int  humid;

//==============

void setup() {

    //Serial.begin(9600);

    //Serial.println("SimpleRxAckPayload Starting");
    radio.begin();
    radio.setDataRate( RF24_250KBPS );
    radio.openReadingPipe(1, thisSlaveAddress);
    radio.setChannel(108);
    radio.enableAckPayload();
    
    radio.startListening();


    pinMode (heatPin, INPUT);
    pinMode (coolPin, INPUT);
    pinMode (fanPin, INPUT);
    pinMode (ductPin, OUTPUT);
    pinMode (humidPin, OUTPUT);
    am2315.begin();

    //delay(1000);
}

//==========

void loop() {
  readSensors();
  radio.writeAckPayload(1, &ackData, sizeof(ackData));
  getData();
  

      
      /*Serial.println("ductState\tsetRH\tsysState\tductState");
      Serial.print(controlData[0]);
      Serial.print("\t\t");
      Serial.print(controlData[1]);
      Serial.print("\t");
      Serial.print(systemState);
      Serial.print("\t\t");
      Serial.println(ductState);
  */
  digitalWrite(ductPin, ductState);
  digitalWrite(humidPin, humidState);
  delay(500);
}

//============

void getData() {
    if ( radio.available() ) {
        radio.read( &controlData, sizeof(controlData) );
        
        newData = true;
    }
}

//================

void readSensors() {
    

   currentMillis = millis();
   if (currentMillis - prevMillis >= senseIntervalMillis) {
   am2315.readSensor();
   humidity = am2315.getHumidity();
   humid= round(humidity);   
   }
   pressure = analogRead(sensorPin);   
   heatState = digitalRead(heatPin);
   int fanRead = digitalRead(fanPin);
   coolState = digitalRead(coolPin);
    if (heatState == 0) systemState = 1;       
     else if (coolState == 0) systemState = 0;    
     else systemState = 2;
    if (controlData[0] == 0 && systemState < 2) {
        ductState = LOW;
    }
     else ductState = HIGH;

    if (humid < controlData[1] && systemState == 1) {
     humidState = HIGH;
    }        
     else humidState = LOW;
    if (fanRead == 0) fanState = 1;
     else fanState = 0;

    ackData[0] = humid; 
    ackData[1] = pressure;
    ackData[2] = systemState; 
    ackData[3] = fanState;
    ackData[4] = humidState;
}
