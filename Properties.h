#ifndef PROPERTIES_H
#define PROPERTIES_H

/*
  System properties หรือ System variable จะเก็บไว้ 2 แบบ ได้แก่
  -ข้อมูลพวกการตั้งค่าการทำงานพื้นฐาน เช่น โหมดการทำงาน,หมายเลขการเชื่อมต่อระบบเครือข่าย ข้อมูล username password ของ MQTT Broker และข้อมลอื่นๆ ซึ่งเป็นข้อมูลที่มีปริมาณไม่มาก จะเก็บไว้ในส่วนที่ถูกกันไว้เป็น EEPROM
  -ข้อมูลพวกการตั้งเวลาเปิด/ปิดหรือข้อมูลอื่นๆ ที่มีปริมาณมากขึ้นมาหน่อยจะถูกเก็บไวใน Flash Memories ส่วนที่เหลือซึ่งถูกกันไว้ให้ทำหน้าที่เป็นเสมือน Hard Disk

  ซึ่งพื้นที่เก็บข้อมูลทั้ง 2 แบบของ ESP32 มันคือส่วนหน่วยความจำแฟลช (SPIFFS) ที่เหลือจากการสงวนไว้(EEPROM) และเหลือใช้จากการเก็บโปรแกรมของเรานั่นเอง
  (หน่วยความจำ Flash ของ ESP32 มีขนาด 4MB สูงสุดคือ 16MB แล้วแต่รุ่น)
*/


#include <TimeLib.h>
#include <LinkedList.h>

#include <ArduinoJson.h>
#include <Preferences.h>

Preferences properties;

//needed for library
//isn't original library. more info https://github.com/cchian/WiFiManagerWithMicrogear (WiFiManager เวอร์ชันปรับปรุง โดย วิเชียร โตโส)
//#include <WiFiManager.h>
//#include <WiFiClient.h> //N/A


#define SERIAL_BAUD_RATE      115200    //Serial_Baud_Rate
#define HTTP_PORT             80        //HTTP port for WebServer
#define TCP_PORT              1988      //TCP port for WebSocket
#define DEVICE_NAME           "WiiLighting-OS" //HostName

#define ACTIVE_LOW 0
#define ACTIVE_HIG 1
#define WORKING_STATE ACTIVE_LOW

#define SPACIFIG_MIN 0
#define SPACIFIG_MAX 1023

//uncomment below to use Arduino Board as GPIO Pins
//#define ARDUINO_AS_IO

int wifistartuptimeout = 2; // in seconds
int pwmFrequency = 3500;    // 3.5 x 10^3 = 3.5KHz (freq limits depend on resolution)
int pwmResolution = 10;     // 10 bit (can between 1 - 16)

//if not include WiFiManager.h use below for Microgear parameter and WiFi Configuration.
#ifndef WiFiManager_h

//-- for developer to connect to WiFi directly--------
String ssid     = "iGateWay";
String password = "34567890";

Preferences WMProp;
boolean canrungear = false;
boolean clearSetting=false;

struct MicrogearParameter {
  char AppID[32];
  char Key[32];
  char Secret[32];
  char Alias[32];
};

MicrogearParameter mgear{
  "WiiLighting",
  "JauQjG7vF4wATlt",
  "i5tRPQBA0g8F2cIr8k6zUdebx",
  "Pim"
};

#endif

#define temperature_sensor_pin 34
int temperature_sensor;

#define maxPWM 1023
#define maxVoltage 6.2



extern TaskHandle_t fadeControlTask; // Task 5 declared on WiiLighting-OS at line 531
extern String splitString(String data, char separator, int index); //declared on MyFunction.h line 35

long timezone = 6;
byte daysavetime = 1;

//จำกัดรายการตั้งเวลาทำงานไว้ที่ 30 รายการ
#define ACCTIME_MAX 30
#define ACCTIME_STORAGE_USED 3072 //พื้นที่จัดเก็บการตั้งเวลาทำงานไม่ให้เกิน 3KB

#include "FS.h"
#include "SPIFFS.h"

#define PATH "times" // for save the actionList to Storage.

String jsonNodeTemplate = F("{'node':'xx','msg':'xxxx','data':'3FF'}");
String jsonActionTemplate = F("{'time':'0000000010','node':'255','data':'3FF'}");
String jsonActionTemplateIncludeNodeName = F("{'time':'0000000010','node':'255','nodeName':'Wii01','data':'3FF'}");

class Action {
  public:
    time_t t;//utc time in second tick started from Jan 01,1970 00:00:00
    String nodeNames; //indef of dev ( dev[i] )
    String lightValue;// 10bit (0-1023)
    Action() {};
    Action(time_t t, String nodeName, String lightValue) {
      this->t = t;
      this->nodeNames = nodeName;
      this->lightValue = lightValue;
    };
};

LinkedList<Action*> actionList = LinkedList<Action*>();

void readTimes() {
  properties.begin(DEVICE_NAME, false);
  actionList = LinkedList<Action*>();
  /*
    File file = SPIFFS.open(PATH);
    if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
    }*/
  String times = properties.getString(PATH);
  int ac_size = properties.getUInt("ac_size");
  if (times.equals("") || ac_size == 0)return;
  for (int i = 0; i < ac_size; i++) {
    String strTime_s = splitString(times, '\n', i);
    String strTime_t = splitString(strTime_s, ' ', 0);
    String strNodeNames = splitString(strTime_s, ' ', 1);
    String strLightValue = splitString(strTime_s, ' ', 2);
    Action *acr = new Action(strTime_t.toInt(), strNodeNames, strLightValue);
    actionList.add(acr);
  }
  Serial.println("Read........ config");
  properties.end();
  /*
    while (file.available()) {
    char fChar = (char)file.read();
    Serial.println(fChar);
    if (fChar == '\n') {
      text.trim();
      String strTime_t = splitString(text, ' ', 0);
      String strNodeNames = splitString(text, ' ', 1);
      String strLightValue = splitString(text, ' ', 2);
      Action *acr = new Action(strTime_t.toInt(), strNodeNames, strLightValue);
      //String str = String(acr->t) + " " + acr->nodeNames + " " + acr->lightValue;
      //Serial.println(str);
      actionList.add(acr);
      text = "";
    }
    text += fChar;
    }

    Serial.println("Read........ config");
    file.close();*/
};

void writeTimes() {
  properties.begin(String(DEVICE_NAME).c_str(), false);
  /*
    File file = SPIFFS.open(PATH, FILE_WRITE);
    if (!file) {
    Serial.println("- failed to open file for writing");
    return;
    }*/
  if (actionList.size() == 0)return;
  String text = "";
  for (int i = 0; i < actionList.size(); i++) {
    Action *ac = actionList.get(i);
    text += String(ac->t) + " " + ac->nodeNames + " " + ac->lightValue;
    if (i < actionList.size() - 1) {
      text += "\n";
    }
  }
  properties.putString(PATH, text);
  properties.putUInt("ac_size", actionList.size());
  properties.end();
}

void clearTimes() {
  properties.begin(String(DEVICE_NAME).c_str(), false);
  actionList.clear();
  properties.putString(PATH, "");
  properties.putUInt("ac_size", 0);
  properties.end();
  /*
    if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
    }
    File file = SPIFFS.open(PATH, FILE_WRITE);
    if (!file) {
    Serial.println("- failed to open file for writing");
    return;
    }

    if (!file.print(0)) {
    file.close();
    return;
    }*/
}

void initProperties() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  /*
    Action *ac = new Action();
    Action *ad = new Action(123, "node1", "0000fa");

    ac->t = 000;
    ac->nodeNames = "node0";
    ac->lightValue = "ffffff";

    actionList.add(ac);
    actionList.add(ad);

    char string[600];
    String str = "Hello";
    str.toCharArray(string, strlen(string));
  */

  //writeTimes();
  readTimes();
  for (int i = 0; i < actionList.size(); i++) {
    Action *ac = actionList.get(i);
    String str = String(ac->t) + " " + ac->nodeNames + " " + ac->lightValue;
    Serial.println(str);
  }

  //long time = root[String("time")];
  //root[String("time")] = time;
  //String nodeNames = root["nodeNames"];

  //nodeNames = root["nodeNames"].as<String>();

  //root["nodeNames"] = nodeNames;
  //root[String("node") + "Names"] = String("node") + "1";
  //  if (root["nodeNames"] == nodeNames) {
  //   // ...
  // }

  // Lastly, you can print the resulting JSON to a String
  // String output;
  //root.prettyPrintTo(output);
  // Serial.println(output);
}

//----------------------------------------------------------------

#ifdef ESP32
HardwareSerial hSerial2(2); // used GPIO16 (RX1), GPIO17 (TX1)
#endif

#define FADE_FUNCTION_ON 1
#define FADE_FUNCTION_OFF 0
boolean fading = true;
int fadingInterval = 2;

boolean canRungear      =     false;
boolean stringComplete  =     false;    // whether the string is complete on Serial
String inputString      =     "";       // a String to hold incoming data from Serial

unsigned long previousMillis = 0, pMillis = 0, pRunning = 0;
const long interval = 5000;
unsigned int nextTime = 0;

/*esp มีหน้าที่อัพเดท STA ไปให้บอร์ด arduino ไม่ให้ arduino เช็คว่า STA มีค่าเป็น 0
  บอร์ด Arduino จะวิเคราะห์ ถ้า STA มีค่า เป็น 0 แสดงว่า esp8266 ไม่ยอมส่งสวยหรือบกพร่องในการทำงาน
  Arduino ก็จะทำการประหาร esp8266 ทันที เพื่อให้ esp8266 เกิดใหม่หรือทำงานเป็นปกติเหมือนเดิมนั่นเอง
*/
int sta_running_duration = (1000 * 60) * 5; // 5 นาที


typedef struct Device {
  int id;        //แชนแนลไอดี ไว้เก็บอินเด็กซ์ของ
  PGM_P label;   //ชื่อที่จะให้แสดง
  String msg;    //สงวนไว้ใช้ในอนาคต (ยังไม่ได้ใช้)
  int  cursta;   //สถานะปัจจุบันของอุปกรณ์
  int  cursta_tmp; //buffer สำหรับฟังก์ชัน fading
  byte pins[4];  //ขาสัญญาณทั้งหมดที่ใช้ในแต่ละแชนแนล (รองรับได้ไม่เกิน 4)
};

/*การเรียกชื่อแชลแนลจะเรียงตามการเรียกอินเด็กซ์ของอาเรย์ คือ เริ่มจาก 0
   ดังนั้นในการควบคุมอุปกรณ์ไฟฟ้า Message
   ต้องเขียนในรูปบบตาม >> อ่านได้ในหน้า Readme
*/

#if defined(ARDUINO_AS_IO)
Device dev[7] = {
  //{label,  msg, cursta, pin_used}
  {0, "Front",   0,    0,     2}, //ใช้ขา 2
  {0, "Inside",  0,    0,     3}, //     3
  {0, "Back",    0,    0,     4}, //     4
  {0, "Toilet",  0,    0,     5}, //     5
  {0, "BedRoom", 0,    0,    12}, //    12
  {0, "FAN-A",   0,    0,    {6,  7, 8}}, // 6 7 8 สำหรับพัดลมหรืออุปกรณ์ที่ปรับความเร็วได้ 3 ระดับ L/M/H (ต้องใช้ขาสัญญาณ 3 ขา)
  {0, "Flash Light 2D", 0,    0,   {22, 23}}  // 22 23 สำหรับพัดลมหรืออุปกรณ์ที่ปรับความเร็วได้ 2 ระดับ L/H (ที่ต้องใช้ขาสัญญาณ 2 ขา)
};
#else
const char naabaan[10] PROGMEM  = { //"ไฟตาแมว"
  0x43,0x6F,0x6F,0x6C,0x57,0x68,0x69,0x74,0x65,0x00
};
const char hero_fan[31] PROGMEM   = {//"พัดลมฮีโร่"
  0xE0, 0xB8, 0x9E, 0xE0, 0xB8, 0xB1, 0xE0, 0xB8, 0x94, 0xE0, 0xB8, 0xA5, 0xE0, 0xB8, 0xA1,
  0xE0, 0xB8, 0xAE, 0xE0, 0xB8, 0xB5, 0xE0, 0xB9, 0x82, 0xE0, 0xB8, 0xA3, 0xE0, 0xB9, 0x88,
  0x00
};
char *rongrean = "\u0e42\u0e23\u0e07\u0e40\u0e23\u0e35\u0e22\u0e19\u0e1a\u0e49\u0e32\u0e19\u0e44\u0e1c\u0e48";
Device dev[1] = {
  //หลีกเลี่ยงการใช้ขา D3
  //not working with pin D3, D3 mapped to pin GPIO0, 0 หรือ null byte ถูกใช้เป็นรหัสปิดท้ายข้อมูลในหน่วยความจำ
  //warning! D4 reserve for LED Indicator on ESP8266Webserver
  /*{    label,  msg, cursta,  cursta_tmp,  pin_used } */
  {0,  naabaan,   "",    0,        0,             4}
 /* ,{0, "LIGHT1",   "",    0,        0,            15},
  {0, "LIGHT2",   "",    0,        0,            25},
  {0, rongrean,   "",    0,        0,           {12 , 27, 14}},
  {0, "LIGHT3",   "",    0,        0,            33}*/
};
#endif

//ตำแหน่ง EEPROM ที่จะให้กู้คืนค่าสถานะขาสัญญาณ
#define PREVIOS_STATUS_ADDRESS 2

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
//สำหรับนำไปทริกขารีเซ็ตของ ESP8266
#define RESET_TRICKER  10
Button resetSwitch(18, DIO);
//สำหรับนำไปทริกขาเคลีร์ยการตั้งค่า
#define CLEAR_SETTING_TRICKER  11

int STA = 1;
bool espboot = false;
#include "pitches.h"
#else

//Clear Setting Tricker,common use with Reset pin
#ifdef ESP8266
#define CLEAR_SETTING_TRICKER 0  //pin D0 for ESP8266
#elif defined(ESP32)
#define CLEAR_SETTING_TRICKER T1  //pin GPO0 for ESP32
#endif

#endif
Button systemResetButton(CLEAR_SETTING_TRICKER, DIO);

#if defined(ARDUINO_ESP8266_NODEMCU) || defined(ESP32)

#include <WebSocketsServer.h>
//for socket
//WiFiServer socketServer(TCP_PORT);
//WiFiClient socketClient;
WebSocketsServer webSocket = WebSocketsServer(TCP_PORT);

#include <MicroGear.h>

//WiFiManager
//Local intialization. Once its business is done, there is no need to keep it around
#ifdef WiFiManager_h
WiFiManager wifiManager;
#else
#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <ESP32WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#endif
#endif
WiFiClient mgearClient;
#include "TimeManager.h"
#include "MyFunction.h"
extern void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
//for GUI Webpage
#ifdef ESP8266
ESP8266WebServer webServer(HTTP_PORT);
#else
ESP32WebServer webServer(HTTP_PORT);
#endif
//for background Service
MicroGear microgear(mgearClient);
DynamicJsonBuffer json;
#endif
#endif


