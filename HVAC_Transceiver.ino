

// SimpleTxAckPayload - the master or the transmitter

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include<Wire.h>
#define CE_PIN   10
#define CSN_PIN 9 

const byte slaveAddress[5] = {'S','E','N','D','R'};

RF24 radio(CE_PIN, CSN_PIN);


int controlData[2] = {1, 35};
int ackData[5];
bool newData = false;

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 1000;

int j = 0;
byte x;
int y;

//===============

void setup() {
      
    Serial.begin(9600);    
    Serial.println("SimpleTxAckPayload Starting");

    radio.begin();
    radio.setDataRate( RF24_250KBPS );
    radio.setChannel(108);
    radio.enableAckPayload();

    radio.setRetries(5,15); // delay, count
                                       // 5 gives a 1500 µsec delay which is needed for a 32 byte ackPayload
    radio.openWritingPipe(slaveAddress);

    Wire.begin(0x08);
    
    Wire.onReceive(receiveEvent);
    Wire.onRequest(sendEvent);
    
}
    
//=============

void loop() {

    currentMillis = millis();
    if (currentMillis - prevMillis >= txIntervalMillis) {
        send();
    }
    //showData();
}

//================

void send() {

    bool rslt;
    rslt = radio.write( &controlData, sizeof(controlData) );
    
    if (rslt) {
        if ( radio.isAckPayloadAvailable() ) {
            radio.read(&ackData, sizeof(ackData));
            newData = true;
        }
        else {
            Serial.println("  Acknowledge but no data ");
        }
        
    }
    else {
        Serial.println("  Tx failed");
    }

    prevMillis = millis();
 }
 

//=================

void showData() {
    if (newData) {
      Serial.println("RH\tPSI\tsysS\tfanS\tRHC\tDstate\tsetRH");
      for (int i=0; i<4;i++) {
        Serial.print(ackData[i]);
        Serial.print("\t");
    }
      Serial.print(ackData[4]);
      Serial.print("\t");
      Serial.print(controlData[0]);
      Serial.print("\t");
      Serial.println(controlData[1]);
      newData = false;
    }
}

//================

  void sendEvent(int howMany)
  {
  for (int i = 0; i < (sizeof(ackData)) / 2; i ++ )
  {
    Wire.write(highByte(ackData[i]));
    Wire.write(lowByte(ackData[i]));
  }
    
  }

//================

void receiveEvent(int howMany) //howMany is always equal to bytes receved
{
  for (int i = 0; i < howMany/2; i++)
  {
    x = Wire.read();
    j++;
    if (j == 1)
    {
      y = (x<<8)|Wire.read();
      controlData[i] = y;
      //Serial.print(y);
      //Serial.print(" ");
      j=0;
    }
  }

  
}
