#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <Adafruit_AM2315.h>
#include <FastLED.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 6
#define BRIGHTNESS 10
#define LED_PIN 10
#define NUM_LEDS 1
#define ACTectionRange 20
#define VREF 3.3
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long senseIntervalMillis = 2000;

Adafruit_AM2315 am2315;
CRGB leds[NUM_LEDS];
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
#define RF95_FREQ 915.0
RH_RF95 rf95(RFM95_CS, RFM95_INT);

struct DataRX
{
  uint16_t Amps;
  uint16_t pressure;  
  uint16_t ODtemp;
  uint8_t duct_RH;
  uint8_t systemState;
  uint8_t RH_Set;
  bool humidControl;
  bool negSign;
} ControlData = {0, 0, 0, 0, 2, 35, false, false};

struct DataTX
{
  bool ductControl;
  uint8_t humidSet;
} MonitorData = {false, 35};
float TempF;
float Amps = 3.1;
const int sensorPin = A1;
const int AmpPin = A2;
const int coolPin = 13;
const int heatPin = 12;
const int fanPin = 11;
const int humidPin = 9;
const int ductPin = 5;

bool heatState;
bool coolState;
bool fanState;
bool fanControl;
//==============
void setup() {
  Serial.begin(9600);
  // OneWire and AM2315 sensors
  sensors.begin();
  am2315.begin();
  pinMode(RFM95_RST, OUTPUT);
  pinMode (heatPin, INPUT);
  pinMode (coolPin, INPUT);
  pinMode (fanPin, INPUT);
  pinMode (ductPin, OUTPUT);
  pinMode (humidPin, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  FastLED.addLeds<WS2812, LED_PIN, RGB>(leds, NUM_LEDS);
  leds[0] = CRGB(0, 255, 0);
  FastLED.show();
    // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(200);
  leds[0] = CRGB(0, 0, 0);
  FastLED.show();
  rf95.init();
  rf95.setFrequency(RF95_FREQ);
  rf95.setTxPower(23, false);


}
//==========
void loop() {
  readSensors();
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      memcpy(&MonitorData, buf, sizeof(MonitorData));
      // Send a reply
      rf95.send((uint8_t *)&ControlData, sizeof(ControlData));
      rf95.waitPacketSent();
    }
  }
  

  //if (newData) {
//    showData();
    //newData = false;
    //}
 
  FastLED.show();
  digitalWrite(ductPin, MonitorData.ductControl);
  digitalWrite(humidPin, ControlData.humidControl);
//  delay(2000);

}

//============
void readSensors() {
  sensors.requestTemperatures();
  float temperature, humidity;
  currentMillis = millis();
  if (currentMillis - prevMillis >= senseIntervalMillis) {
    prevMillis = currentMillis;
    am2315.readTemperatureAndHumidity(&temperature, &humidity);
    ControlData.duct_RH = round(humidity);
  }
  
    
  heatState = digitalRead(heatPin);
  coolState = digitalRead(coolPin);
  fanControl = digitalRead(fanPin);

  ControlData.pressure = round(analogRead(sensorPin));
  TempF = sensors.getTempFByIndex(0);
  if(TempF < 0) {
    ControlData.negSign = true;
    TempF = abs(TempF);
  }
  else ControlData.negSign = false;
  ControlData.ODtemp = round(TempF * 10.0);
  Amps = readACCurrentValue();
  ControlData.Amps = round(Amps * 10.0);
  
//  if (!fanControl) Amps = 6.2;
//  else Amps = 4.1;
  
  if (!heatState) {
    ControlData.systemState = 1;
    //      lastMode = 1;
    leds[0] = CRGB(0, 255, 0);
  }
  else if (!coolState) {
    ControlData.systemState = 0;
    //     lastMode = 0;
    leds[0] = CRGB(0, 0, 255);
  }
  else {
    ControlData.systemState = 2;
    leds[0] = CRGB(255, 0, 0);
  }

  if (Amps > 4.5) fanState = true;
  else fanState = false;
  
  if (MonitorData.humidSet > 0) ControlData.RH_Set = MonitorData.humidSet;
  else if (TempF <= 20) ControlData.RH_Set = round(50 - ((50 - ControlData.ODtemp) / 2));
  else ControlData.RH_Set = 35;
  if ((ControlData.duct_RH < MonitorData.humidSet) && ControlData.systemState == 1 && fanState)
  {
    ControlData.humidControl = true;
  }
  if (ControlData.duct_RH >= MonitorData.humidSet || !fanState) {
    ControlData.humidControl = false;
  }

}
//============
void showData() {
  Serial.println("RH%\tPSI\tTempF\tAmps\tTx10\tAmpsx10");
  Serial.print(ControlData.duct_RH);
  Serial.print("\t");
  Serial.print(ControlData.pressure);
  Serial.print("\t");
  Serial.print(TempF);
  Serial.print("\t");
  Serial.print(Amps);
  Serial.print("\t");
  Serial.print(ControlData.ODtemp);
  Serial.print("\t");
  Serial.println(ControlData.Amps);
  }
//==============
float readACCurrentValue()
{
  float ACCurrentValue = 0;
  float peakVoltage = 0;
  float voltageVirtualValue = 0;  //Vrms
  for (int i = 0; i < 5; i++)
  {
    peakVoltage += analogRead(AmpPin);   //read peak voltage
    delay(1);
  }
  peakVoltage = peakVoltage / 5.0;
  voltageVirtualValue = peakVoltage * 0.707;

  /*The circuit is amplified by 2 times, so it is divided by 2.*/
  voltageVirtualValue = (voltageVirtualValue / 1024 * VREF ) / 2.0;

  ACCurrentValue = voltageVirtualValue * ACTectionRange;

  return ACCurrentValue;
}
