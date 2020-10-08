
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Adafruit_seesaw.h>
#include <Adafruit_TFTShield18.h>
#include <DHT.h>
#include <SD.h>

//Color Definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF
#define ORANGE   0xFD20

#define DS3231_I2C_ADDRESS 0x68

#define DHTTYPE DHT22
#define DHTPIN 24

#define SD_CS    4  // Chip select line for SD card on Shield
#define TFT_CS  10  // Chip select line for TFT display on Shield
#define TFT_DC   8  // Data/command line for TFT on Shield
#define TFT_RST  -1  // Reset line for TFT is handled by seesaw!

int j = 0;
byte x;
int y;

int controlData[2] = {1, 35};
int ackData[5] = {0, 0, 2, 0, 0};

byte second, minute, hour, day, month, year;
File logFile;

unsigned long nowMillis;
unsigned long beginMillis;
unsigned long endTime;
const unsigned long longTime = 300000;
const unsigned long shortTime = 30000;
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 2000;
float rh;
float f;
int RH;
int F;

uint32_t buttons;
uint8_t systemState = 2;
uint8_t fanState;
uint8_t humidState;
uint8_t ductState = 1;
uint8_t lastMode;
unsigned int pressure;
unsigned int humid;

bool Set = false;
String modeString[] = {"COOL", "HEAT"};
String setString[] = {"COOL     ", "HEAT    ", "HUMIDITY"};
int setValue[3] = {77, 70, 35};
int i = 1;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_TFTShield18 ss;

DHT dht(DHTPIN, DHTTYPE);

byte bcdToDec(byte val){
  return( (val/16*10) + (val%16) );
}

byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}

void setup() {  
  
  Serial.begin(9600);

  Wire.begin();  
  
  dht.begin();

  startMillis = millis();

  // start by disabling both SD and TFT
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // Start seesaw helper chip
  if (!ss.begin()){
    Serial.println("seesaw could not be initialized!");
    while(1);
  }
  Serial.println("seesaw started");

  // Start set the backlight off
  ss.setBacklight(TFTSHIELD_BACKLIGHT_OFF);
  // Reset the TFT
  ss.tftReset();

  // Initialize 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  Serial.println("TFT OK!");
  tft.setRotation(1);
  ss.setBacklight(TFTSHIELD_BACKLIGHT_ON);
  tft.fillScreen(BLACK);

  if (!SD.begin(SD_CS)) {
    Serial.println("SD failed!");
    //return;
  }
  Serial.println("Card Ready");

  logFile = SD.open("LOG.csv", FILE_WRITE);
  if (logFile) 
  {
    logFile.println("Date, Time, RH, Pressure, systemState, fanState, HumidState, DuctRH, ductState");
    logFile.close();
  }
  else Serial.println("Couldn't open log file");

  delay(500);
  

}

void loop() {

  
  Wire.requestFrom(0x08, 10);
  if(Wire.available()) {    
  
  for (int k = 0; k < 5; k++) {
    x = Wire.read();
    j++;
    if (j == 1) {
      y = (x << 8) | Wire.read();
      ackData[k] = y;
      j = 0;
    }
  }
  }
  
  bool newState = false;
  bool newData = false;

  

    if (humidState != ackData[4]){
      humidState = ackData[4];
      newData = true;
    }
      
    if (systemState != ackData[2]){
      
      systemState = ackData[2];
      
      newState = true;
      newData = true;
    }
    if (fanState != ackData[3]){
      fanState = ackData[3];
      newData = true;
    }
    
    humid = ackData[0];
    pressure = ackData[1];

     
  currentMillis = millis();
  if (currentMillis - startMillis >= period){
  rh = dht.readHumidity();
  f = dht.readTemperature(true);
  RH= round(rh);
  F = round(f);    
  startMillis = currentMillis;
  }
  
  ductState = 1;

  if(systemState == 0 && setValue[0] < F) ductState = 0;
    else if(systemState == 1 && setValue[1] > F) ductState = 0;
    
    controlData[0] = ductState;
    controlData[1] = setValue[2];
    
    if(newState && systemState < 2) lastMode = systemState;
    
    // Semd to SD card
    nowMillis = millis();    
    if (systemState < 2) {
      endTime = shortTime;
    }
    else endTime = longTime;
    
    if (newData || nowMillis - beginMillis >= endTime)
     {
      logData();
      newData = false;
      beginMillis = nowMillis;
     }
    /*  Serial.println("RH\tPSI\tsysS\tfanS\tRHC\tnewState\tlastMode");
      for (int i=0; i<5;i++) {
        Serial.print(ackData[i]);
        Serial.print("\t");
    }
      Serial.print(newState);
      Serial.print("\t\t");
      Serial.println(lastMode);
    */

  sendData();

  displayTime();
 
  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(0, 13);
  tft.setTextSize(1);
  tft.print("SYSTEM FAN DUCT HUMIDIFIER");
  tft.fillRect(0, 25, 30, 16, BLACK);
  
  if (systemState == 2) {  
  
   if (lastMode == 1) {
    tft.drawRect(0, 25, 30, 16, MAGENTA);
    tft.setTextColor(WHITE, BLACK);
   }
   else {
    tft.drawRect(0, 25, 30, 16, BLUE);   
    tft.setTextColor(WHITE, BLACK);
   }
  }

  else {if (systemState == 1) {
    tft.fillRect(0, 25, 30, 16, MAGENTA);
    tft.setTextColor(WHITE);
   }
   else {
    tft.fillRect(0, 25, 30, 16, BLUE);
    tft.setTextColor(WHITE);
   }
  }
  
    tft.setCursor(4, 29);
    tft.print(modeString[lastMode]);

    tft.setTextColor(WHITE, BLACK);
    if (fanState) {
      tft.fillRect(35, 25, 25, 16, GREEN);
      tft.setTextColor(WHITE);
      tft.setCursor(40, 29);
      tft.print("ON");
    }
    else {
      tft.fillRect(35, 25, 25, 16, BLACK);
      tft.drawRect(35, 25, 25, 16, RED);
      tft.setCursor(40, 29);
      tft.print("OFF");
    }
     tft.setTextColor(WHITE, BLACK);
     if (!ductState) {
      tft.fillRect(65, 25, 25, 16, GREEN);
      tft.setTextColor(WHITE);
      tft.setCursor(70, 29);
      tft.print("ON");
     }
     else {
      tft.fillRect(65, 25, 25, 16, BLACK);
      tft.drawRect(65, 25, 25, 16, RED);
      tft.setCursor(70, 29);
      tft.print("OFF");
     }
     tft.setTextColor(WHITE, BLACK);
     if (humidState) {
      tft.fillRect(96, 25, 35, 16, GREEN);
      tft.setTextColor(WHITE);
      tft.setCursor(105, 29);
      tft.print("ON");
     }
     else {
      tft.fillRect(96, 25, 35, 16, BLACK);
      tft.drawRect(96, 25, 35, 16, RED);
      tft.setCursor(105, 29);
      tft.print("OFF");
     }
     tft.setTextColor(WHITE, BLACK);
     tft.setCursor(30, 46);
     tft.print("TEMPERATURE");
     tft.setCursor(115, 46);
     tft.print("%RH");
     tft.setCursor(0, 60);
     tft.print("ROOM:");
     tft.setCursor(35, 56);
     tft.setTextSize(2);
     tft.print(F);
     tft.print((char)247);
     tft.print("F");
     tft.setCursor(110, 56);     
     tft.print(RH);
     tft.print("%");
     tft.setCursor(0, 80);
     tft.setTextSize(1);
     tft.print(" SET:");
     tft.setCursor(35, 76);
     tft.setTextSize(2);

     tft.print(setValue[lastMode]);     
     tft.print((char)247);
     tft.print("F");
     tft.setCursor(110, 76);
     tft.print(setValue[2]);
     tft.print("%");     
     tft.setTextSize(1);

     tft.setCursor(0, 98);
     tft.print("DUCT RH");
     tft.setCursor(55, 98);
     tft.print(humid);
     tft.print("%");
     
     tft.setCursor(0, 116);
     tft.print("FILTER pressure:");
     tft.print(pressure);
     tft.drawRect(95, 112, 60, 16, ORANGE);
    
    buttons = ss.readButtons();     

    if(! (buttons & TFTSHIELD_BUTTON_1)) {
      Set = !Set;
      tft.fillScreen(BLACK);
    }

    while(Set) {
      
      tft.setTextSize(2);
      tft.setTextColor(YELLOW, BLACK);
      tft.setCursor(40, 16);
      tft.print("Set Mode");
      tft.setCursor(0, 40);
      tft.setTextColor(WHITE, BLACK);
      tft.print("SET:");
      tft.setCursor(50, 40);      
      tft.print(setString[i]);
      tft.setCursor(50, 57);
      tft.print(setValue[i]);
      if(i < 2) {
      tft.print((char)247);
      tft.print("F");
      }
      else tft.print("%");

      buttons = ss.readButtons();

      if(! (buttons & TFTSHIELD_BUTTON_UP)) setValue[i]++;
      if(! (buttons & TFTSHIELD_BUTTON_DOWN)) setValue[i]--;
      
      if(! (buttons & TFTSHIELD_BUTTON_RIGHT)){
        i++;
        if (i > 2 ) i = 0;
      }
      if(! (buttons & TFTSHIELD_BUTTON_LEFT)){
        i--;
      if (i < 0) i = 2;
      }
      if(! (buttons & TFTSHIELD_BUTTON_3)) {
        
        Set = false;
        tft.fillScreen(BLACK);
      }
      delay(300);
    }
  
   delay(500);  
}

//=============
void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *day,
byte *month,
byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *day = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}
//=============
  void displayTime()
{
  byte second, minute, hour, dayOfWeek, day, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &day, &month,
  &year); 
  
  tft.setTextSize(1);
  tft.setTextColor(BLUE, BLACK);
  tft.setCursor(5, 0);
  tft.print(month);
  tft.print("/");
  tft.print(day);
  tft.print("/");
  tft.print(year);
  tft.print("  ");
  if (hour<10) tft.print("0");
  tft.print(hour);
  tft.print(":");
  if (minute<10) tft.print("0");    
  tft.print(minute);
  tft.print(":");
  if (second<10) tft.print("0");
  tft.print(second);
  }
//=============  
void sendData()
{
  Wire.beginTransmission(0x08);
  for (int i = 0; i <(sizeof(controlData))/2; i ++ ) {
    Wire.write(highByte(controlData[i]));
    Wire.write(lowByte(controlData[i]));   
  }
  Wire.endTransmission();
}
   
//=============    

void logData() {
  
  byte second, minute, hour, dayOfWeek, day, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &day, &month,
  &year);

   logFile = SD.open("LOG.csv", FILE_WRITE);
   logFile.print(month);
   logFile.print("-");
   logFile.print(day);
   logFile.print("-");
   logFile.print(year);
   logFile.print(",");
   logFile.print(hour);
   logFile.print(":");
   logFile.print(minute);
   logFile.print(":");
   logFile.print(second);
   logFile.print(",");
   for(int i = 0; i<5; i++) {
    logFile.print(ackData[i]);
    logFile.print(",");
   }
    logFile.print(humid);
    logFile.print(",");
    logFile.println(ductState);
    logFile.close();
}

   
