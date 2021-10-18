#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Adafruit_seesaw.h>
#include <Adafruit_TFTShield18.h>
#include <Adafruit_AHTX0.h>
#include <SD.h>
#include <RH_RF95.h>
#include <RTClib.h>
#include <avr/wdt.h>
#include <EEPROM.h>
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

#define SYS_LED 32
#define HUMID_LED 23
#define DUCT_LED 25
#define DS3231_I2C_ADDRESS 0x68
#define RFM95_CS 28
#define RFM95_RST 31
#define RFM95_INT 18
#define RF95_FREQ 915.0
#define SD_CS    4  // Chip select line for SD card on Shield
#define TFT_CS  10  // Chip select line for TFT display on Shield
#define TFT_DC   8  // Data/command line for TFT on Shield
#define TFT_RST  -1  // Reset line for TFT is handled by seesaw!

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
} ControlData = {0, 0, 0, 0, 2, 0, false, false};

struct DataTX
{
  bool ductControl;
  uint8_t humidSet;
} MonitorData = {false, 35};

//byte second, minute, hour, day, month, year;
File logFile;

unsigned long nowMillis;
unsigned long beginMillis;
unsigned long endTime;
const unsigned long longTime = 1200000;
const unsigned long shortTime = 20000;
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 2000;
float rh;
float c;
int RH;
int F;
float ODTemp = 99.0;
float Amps = 3.1;
uint32_t buttons;
uint8_t sysState = 2;
bool fanState;
bool humidState;
bool ductState;
bool lastMode;
bool lastFan;
bool lastHumid;
bool Set;
int setting[3] = {75, 70, 35};
int i = 1;
bool DFlag = false;

Adafruit_AHTX0 aht;
Adafruit_TFTShield18 ss;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
RTC_DS3231 RTC;
DateTime now;

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup()
{
  //Uncomment to set time
  //if(rtc.lostPower()) rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  wdt_enable(WDTO_4S);
  //  Serial.begin(9600);
  RTC.begin();
  Wire.begin();
  aht.begin();
  startMillis = millis();
  pinMode(HUMID_LED, OUTPUT);
  pinMode(DUCT_LED, OUTPUT);
  pinMode(SYS_LED, OUTPUT);
  // Reset and start RF95
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  rf95.init();
  rf95.setFrequency(RF95_FREQ);
  rf95.setTxPower(23, false);
  // start by disabling both SD and TFT
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // Start seesaw helper chip
  ss.begin();

  // Start set the backlight off
  ss.setBacklight(TFTSHIELD_BACKLIGHT_OFF);
  // Reset the TFT
  ss.tftReset();

  // Initialize 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  //  Serial.println("TFT OK!");
  tft.setRotation(1);
  tft.setTextWrap(false);
  ss.setBacklight(TFTSHIELD_BACKLIGHT_ON);
  tft.fillScreen(BLACK);
//If power is lost or WDT reset this will restore last settings
  if (EEPROM.read(3))
  {
    for (int j = 0; j < 3; j++)
    {
      setting[j] = EEPROM.read(j);
    }
  }
  if (SD.begin(SD_CS))
  {
    logFile = SD.open("LOG.csv", FILE_WRITE);
    if (logFile)
    {
      logFile.print("Date,Time,sysState,DuctControl,RoomTemp,FanState,Amps,");
      logFile.println("HumidControl,HumidSet,DuctRH,RoomRH,ODTemp,Pressure,RSSI");
      logFile.close();
    }
  }
  delay(200);

}
void loop()
{

  currentMillis = millis();
  if (currentMillis - startMillis >= period)
  {
    exchangeData();
    startMillis = currentMillis;
  }
  //Determining if there is new data to log:
  bool newState = false;
  bool newData = false;

  if (humidState != ControlData.humidControl)
  {
    humidState = ControlData.humidControl;
    newData = true;
  }
  if (sysState != ControlData.systemState)
  {
    sysState = ControlData.systemState;
    newState = true;
    newData = true;
  }
  if (lastHumid != ControlData.humidControl)
  {
    lastHumid = ControlData.humidControl;
    newData = true;
  }

  if (Amps > 5) fanState = true;
  else fanState = false;

  if (lastFan != fanState)
  {
    lastFan = fanState;
    newData = true;
  }

  //get aht data
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  rh = humidity.relative_humidity;
  c = temp.temperature;
  RH = round(rh);
  F = round(c * 9 / 5 + 32);

  //determine if duct should be on for cooling or heating
  if (sysState == 0 && setting[0] < F) MonitorData.ductControl = true;
  else if (sysState == 1 && setting[1] > F) MonitorData.ductControl = true;
  
  //turn duct off when fan shuts off or when the system is off and thermostat fan is set to on
  if (!fanState || (sysState == 2 && fanState)) MonitorData.ductControl = false;
  
  MonitorData.humidSet = setting[2];
  
  if (newState && sysState < 2) lastMode = sysState;
  digitalWrite(HUMID_LED, ControlData.humidControl);
  digitalWrite(DUCT_LED, MonitorData.ductControl);
  if (sysState < 2) digitalWrite(SYS_LED, HIGH);
  else digitalWrite(SYS_LED, LOW);
  // Send to SD card-long time if system not running
  nowMillis = millis();
  if (sysState < 2) endTime = shortTime;
  else endTime = longTime;

  if (newData || (nowMillis - beginMillis >= endTime))
  {
    if (SD.exists("LOG.csv")) logData();
    newData = false;
    beginMillis = nowMillis;
  }
  displayTime();

  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(0, 13);
  tft.setTextSize(1);
  tft.print("SYSTEM FAN DUCT HUMIDIFIER");
  tft.fillRect(0, 25, 30, 16, BLACK);

  if (sysState == 2)
  {

    if (lastMode == 1)
    {
      tft.drawRect(0, 25, 30, 16, ORANGE);
      tft.setTextColor(WHITE, BLACK);
    }
    else
    {
      tft.drawRect(0, 25, 30, 16, BLUE);
      tft.setTextColor(WHITE, BLACK);
    }
  }

  else {
    if (sysState == 1)
    {
      tft.fillRect(0, 25, 30, 16, ORANGE);
      tft.setTextColor(BLACK);
    }
    else
    {
      tft.fillRect(0, 25, 30, 16, BLUE);
      tft.setTextColor(BLACK);
    }
  }
  tft.setCursor(4, 29);
  if (lastMode) tft.print("HEAT");
  else tft.print("COOL");

  tft.setTextColor(WHITE, BLACK);
  if (fanState)
  {
    tft.fillRect(35, 25, 25, 16, GREEN);
    tft.setTextColor(BLACK);
    tft.setCursor(40, 29);
    tft.print("ON");
  }
  else
  {
    tft.fillRect(35, 25, 25, 16, BLACK);
    tft.drawRect(35, 25, 25, 16, RED);
    tft.setCursor(40, 29);
    tft.print("OFF");
  }
  tft.setTextColor(WHITE, BLACK);
  if (MonitorData.ductControl)
  {
    tft.fillRect(65, 25, 25, 16, GREEN);
    tft.setTextColor(BLACK);
    tft.setCursor(70, 29);
    tft.print("ON");
  }
  else
  {
    tft.fillRect(65, 25, 25, 16, BLACK);
    tft.drawRect(65, 25, 25, 16, RED);
    tft.setCursor(70, 29);
    tft.print("OFF");
  }

  if (humidState)
  {
    tft.fillRect(96, 25, 35, 16, GREEN);
    tft.setTextColor(BLACK);
    if (setting[2] == 0)
    {
      tft.setCursor(100, 29);
      tft.print("AUTO");
    }
    else
    {
      tft.setCursor(105, 29);
      tft.print("ON");
    }
  }
  else
  {
    tft.setTextColor(WHITE, BLACK);
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
  tft.print(setting[lastMode]);
  tft.print((char)247);
  tft.print("F");
  tft.setCursor(110, 76);
  tft.print(ControlData.RH_Set);
  tft.println("%");

  tft.setTextSize(1);
  tft.setCursor(0, 94);
  tft.print("ODTemp: ");
  tft.setCursor(48, 94);
  if (abs(ODTemp < 10)) tft.print(" ");
  else if (abs(ODTemp < 100)) tft.print(" ");
  if (ODTemp < 0) tft.print("-");
  else tft.print(" ");
  tft.print(ODTemp, 1);
  tft.print((char)247);
  tft.println("F");
  //Determine if regular or alternate lower display
  DFlag = EEPROM.read(4);
  if (DFlag)
  {
    tft.setCursor(0, 106);
    tft.print("DuctRH: ");
    tft.setCursor(54, 106);
    if (ControlData.duct_RH < 10) tft.print(" ");
    tft.print(ControlData.duct_RH);
    tft.print("%");
    tft.setCursor(0, 120);
    tft.print("FILTER pressure:");
    tft.println(ControlData.pressure);
  }
  else

  {
    tft.setCursor(0, 106);
    tft.print("AMPS: ");
    tft.setCursor(48, 106);
    if (Amps < 10) tft.print(" ");
    tft.println(Amps, 1);
    tft.setCursor(0, 120);
    tft.print("RSSI: ");
    tft.setCursor(48, 120);
    tft.println(rf95.lastRssi(), DEC);
  }
  
  buttons = ss.readButtons();
  //Enter setting mode
  if (! (buttons & TFTSHIELD_BUTTON_1))
  {
    Set = true;
    tft.fillScreen(BLACK);
  }
  while (Set)
  {
    tft.setTextSize(2);
    tft.setTextColor(YELLOW, BLACK);
    tft.setCursor(40, 16);
    tft.print("Set Mode");
    tft.setCursor(0, 40);
    tft.setTextColor(WHITE, BLACK);
    tft.print("SET:");
    tft.setCursor(50, 40);
    if (i == 1) tft.print("HEAT    ");
    else if (i == 0) tft.print("COOL     ");
    else tft.print("HUMIDITY");
    tft.setCursor(50, 58);
    tft.print(setting[i]);
    if (i < 2)
    {
      tft.print((char)247);
      tft.print("F");
    }
    else tft.print("%");
    tft.setTextSize(1);
    tft.setTextColor(YELLOW, BLACK);
    tft.setCursor(50, 80);
    tft.print("Bottom Display");
    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(2);
    tft.setCursor(50, 95);
    if (DFlag) tft.print("Regular  ");
    else tft.print("Alternate");

    buttons = ss.readButtons();
    //Change settings with up and down
    if (! (buttons & TFTSHIELD_BUTTON_UP)) setting[i]++;
    if (! (buttons & TFTSHIELD_BUTTON_DOWN)) setting[i]--;
    if (setting[i] < 0) setting[i] = 0;
    //Change which setting with right and left
    if (! (buttons & TFTSHIELD_BUTTON_RIGHT))
    {
      i++;
      if (i > 2 ) i = 0;
    }
    if (! (buttons & TFTSHIELD_BUTTON_LEFT))
    {
      i--;
      if (i < 0) i = 2;
    }
    //Change lower display
    if (! (buttons & TFTSHIELD_BUTTON_2))
    {
      DFlag = !DFlag;
    }
    //Exit settings and update EEPROM
    if (! (buttons & TFTSHIELD_BUTTON_3))
    {
    // update settings and leave setting mode
      Set = false;
      for (int j = 0; j < 3; j++)
      {
        EEPROM.update(j, setting[j]);
      }
      EEPROM.update(3, 1);
      EEPROM.update(4, DFlag);
      tft.fillScreen(BLACK);
    }

    delay(200);
    wdt_reset();
  }
  wdt_reset();
  delay(500);
}
//============
void exchangeData()
{
  rf95.send((uint8_t *)&MonitorData, sizeof(MonitorData));
  delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  delay(10);

  if (rf95.waitAvailableTimeout(1000))
  {
    // Should be a reply message for us now
    if (rf95.recv(buf, &len))
    {
      memcpy(&ControlData, buf, sizeof(ControlData));
      ODTemp = ControlData.ODtemp / 10.0;
      if (ControlData.negSign) ODTemp = -ODTemp;
      Amps = ControlData.Amps / 10.0;
    }
  }
}

//=============
void displayTime()
{
  now = RTC.now();
  tft.setTextSize(1);
  tft.setTextColor(YELLOW, BLACK);
  tft.setCursor(10, 0);
  tft.print(now.month(), DEC);
  tft.print("/");
  tft.print(now.day(), DEC);
  tft.print("/");
  tft.print(now.year(), DEC);
  tft.print("  ");
  if (now.twelveHour() < 10) tft.print("0");
  tft.print(now.twelveHour(), DEC);
  tft.print(":");
  if (now.minute() < 10) tft.print("0");
  tft.print(now.minute(), DEC);
  tft.print(":");
  if (now.second() < 10) tft.print("0");
  tft.print(now.second(), DEC);
  if (now.isPM()) tft.print(" PM");
  else tft.print(" AM");
}

//=============
void logData()
{
  now = RTC.now();
  logFile = SD.open("LOG.csv", FILE_WRITE);
  logFile.print(now.month(), DEC);
  logFile.print("-");
  logFile.print(now.day(), DEC);
  logFile.print("-");
  logFile.print(now.year(), DEC);
  logFile.print(",");
  logFile.print(now.hour(), DEC);
  logFile.print(":");
  logFile.print(now.minute(), DEC);
  logFile.print(":");
  logFile.print(now.second(), DEC);
  logFile.print(F(","));
  logFile.print(sysState);
  logFile.print(",");
  logFile.print(MonitorData.ductControl);
  logFile.print(",");
  logFile.print(F);
  logFile.print(",");
  logFile.print(fanState);
  logFile.print(",");
  logFile.print(Amps);
  logFile.print(",");
  logFile.print(ControlData.humidControl);
  logFile.print(",");
  logFile.print(ControlData.RH_Set);
  logFile.print(",");
  logFile.print(ControlData.duct_RH);
  logFile.print(",");
  logFile.print(RH);
  logFile.print(",");
  logFile.print(ODTemp);
  logFile.print(",");
  logFile.print(ControlData.pressure);
  logFile.print(",");
  logFile.println(rf95.lastRssi(), DEC);
  logFile.close();
}
