//  WiiLighting-OS v4.0 Lignting Control firmware for ESP32 Copy Left by Wichian Toso
//  (this sketch was available on https://github.com/cchian/wiilighting-os)

//  Change Arduino editor font to "Courier New" for specify correct ESP32 image below.
//  1.Click the link at the line following File > Preferences > More preferences can be edited directly in the file.
//    This will open the .Arduino15 (or something like that) folder.
//  2.Exit the Arduino IDE
//  3.Open the file preferences.txt in a text editor.
//  4.Search for the line that starts with "editor.font".
//  5.Change the first word following the equals sign on that line to the "Courier New".
//    like this "editor.font=Courier New,plain,11"
//  6.Save the file.
//  7.Start the Arduino IDE. The editor should now be using the font you chose.
//
//
//
//                                                 ┌─────────────────────────────┐
//                                                 │  ┌───┐   ┌───┐   ┌───┬───┐  │
//                                                 │  │   │   │   │   │   │   │  │
//                                                 │  │   └───┘   └───┘   │   │  │
//                                                 │                      │   │  │
//                                                 │┌───────────────────────────┐│
//                                         GND  1-▓┤│ º                         │├▓-38 GND
//                                         VCC  2-▓┤│                           ░├▓-37 GPIO23
//                                    !     EN  3-▓┤│                           ░├▓-36 GPIO22            SCL
//                  ADC_H ADC1_0 SensVP GPI36   4-▓┤│                           ░├▓-35 GPIO1  TX0        CLK03
//                  ADC_H ADC1_3 SensVN GPI39   5-▓┤│                           ░├▓-34 GPIO3  RX0        CLK02
//                        ADC1_6        GPI34   6-▓┤│                           ░├▓-33 GPIO21            SDA
//                        ADC1_7        GPI35   7-▓┤│           ESP32           ░├▓-32 GPIO20
//                 XTAL32 ADC1_4     T9 GPO32   8-▓┤░                           ░├▓-31 GPIO19
//                 XTAL32 ADC1_5     T8 GPO33   9-▓┤░                           ░├▓-30 GPIO18
//                        ADC2_8  DAC_1 GPIO25 10-▓┤░                           ░├▓-29 GPIO5
//                        ADC2_9  DAC_2 GPIO26 11-▓┤░                           ░├▓-28 GPIO17 TX1
//                        ADC2_7     T7 GPIO27 12-▓┤░                           ░├▓-27 GPIO16 RX1
//                        ADC2_6     T6 GPIO14 13-▓┤░                           ░├▓-26 GPIO4  T0  ADC2_0
//                        ADC2_5     T5 GPIO12 14-▓┤░──────░░░░░░░░░░░░░░░░░────░├▓-25 GPIO0  T1  ADC2_1 CLK01
//                                                 └─────▓─▓─▓─▓─▓─▓─▓─▓─▓─▓─────┘
//                                         GND 15────────┘ │ │ │ │ │ │ │ │ └────────24 GPIO2  T2  ADC2_2
//                        ADC2_4     T4 GPIO13 16──────────┘ │ │ │ │ │ │ └──────────23 GPIO15 T3  ADC2_3
//                        SD_D2          GPIO9 17────────────┘ │ │ │ │ └────────────22 GPIO8      SD_D1
//                        SD_D3         GPIO10 18──────────────┘ │ │ └──────────────21 GPIO7      SD_D0
//                        SD_CMD        GPIO11 19────────────────┘ └────────────────20 GPIO6      SD_CLK
//
//
//
//                                                 ░    PWM Pin
//                                                 1-38 Physical pin on package
//
//                                                 Absolute MAX per pin 12mA, recommended 6mA
//                                                 VCC = 3.3V
//
//
//
//
//
//WiiLighting-OS v1.0 (Code Name:GearDev, support ESP8266 only)        -->>>No Debut
//created date: Nov 01, 2017
//success date: Nov 25, 2017

//WiiLighting-OS v2.0 (Code Name:iot32Smart, new version for ESP32)    -->>>No Debut
//start  date: June 01, 2018
//succes date: July 01, 2018?

//WiiLighting-OS v3.0 (Code Name:WiiLighting-OS, support ESP32 only)   -->>>No Debut
//start  date: July 01, 2018
//succes date: July 31, 2018?

//WiiLighting-OS v4.0 in December 01, 2018 (in trend Thailand4.0)      -->>>Wait for Debut
//visit https://cchian.github.io/wiilighting-os
//visit https://github.com/cchian/wiilighting-os


#include <ArduinoJson.h>
#include <Button.h>
#include "time.h"
//#include <EEPROM.h>

#include "Properties.h"

#if defined(ARDUINO_ESP8266_NODEMCU) || defined(ESP32)

#include "WebContent.h"

void commander(uint8_t* msg, unsigned int msglen) {
  String s = char2String(msg, msglen);
  s.replace("\"", "\"");
  JsonObject& root = json.parseObject(s);
  Serial.print("Cammander "); Serial.print(root["type"].as<String>()); Serial.print(" : "); Serial.println(root["alias"].as<String>());
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  if (clearSetting == true) {
    clearWiFiSetting();
#if defined(ESP8266)
    ESP.reset();
#else
    ESP.restart();
#endif
  }
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.print("reconnect to WiFi");
#ifdef WiFiManager_h
      WiFi.begin(wifiManager.getSSID().c_str(), wifiManager.getPassword().c_str());
#else
      WiFi.begin(ssid.c_str(), password.c_str());
#endif
      break;
  }
}

/* If a new message arrives, do this */
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  String sMsg = "";
  for (int i = 0; i < msglen; i++)
    sMsg += (char)msg[i];
  //process.......................
  String sender = splitString(sMsg, '#', 0);
  String recv = splitString(sMsg, '#', 1);
  String body = splitString(sMsg, '#', 2);
#ifdef WiFiManager_h
  if (!recv.equals(String(wifiManager.mgear.Alias)))return;
#else
  if (!recv.equals(String(mgear.Alias)))return;
#endif
  char bsender[32];
  sender.toCharArray(bsender, 32);
  if (body.startsWith("a")) {
    String strVal = splitString(body, '=', 1);
    if (isValidNumber(strVal)) {
      int intVal = strVal.toInt();
      if (intVal >= 0 && intVal <= 1023) {
        for (int i = 0; i < sizeof(dev) / sizeof(dev[0]); i++) {
          dev[i].cursta_tmp = intVal;
          if (!fading) {
            dev[i].cursta = intVal;
            setPinsValue(i);
          }
        }
      }
      Serial.println("a=" + strVal);
      //uncomment below 1 line if sending data to alias
      //microgear.chat(bsender, String(wifiManager.mgear.Alias) + "#" + String(sender) + "#a=" + strVal);
      //uncomment below 1 line if sending data to topic
#ifdef WiFiManager_h
      microgear.publish("/chat", String(wifiManager.mgear.Alias) + "#" + String(sender) + "#a=" + strVal, true);
#else
      microgear.publish("/chat", String(mgear.Alias) + "#" + String(sender) + "#a=" + strVal, true);
#endif
      //dumpioIndex();
      return;
    }
  } else if (body.startsWith("?")) {
    //    body = "all=";
    //    for (int i = 0; i < sizeof(dev) / sizeof(dev[0]); i++) {
    //
    //      body += String(dev[i].label) + ":" + dev[i].cursta;
    //      if (i < sizeof(dev) / sizeof(dev[0]) - 1)body += ",";
    //    }
    //    Serial.println("?");
    //
    //    //microgear.chat(bsender, String(wifiManager.mgear.Alias) + "#" + String(sender) + "#" + body);
    //    String msg = String(wifiManager.mgear.Alias) + "#" + String(sender) + "#" + body;
    //    Serial.println(msg);
    //    if (microgear.connected()) {
    //      microgear.publish("/chat", msg , true);
    //    }
    //    return;
    //-------------------------------------------------------------------------------------------------------
    DynamicJsonBuffer jsonBuffer;
    JsonObject& sta = jsonBuffer.parseObject(jsonNodeTemplate);
    body = "[";
    for (int i = 0; i < sizeof(dev) / sizeof(dev[0]); i++) {
      sta["node"] = dev[i].label;
      sta["msg"] = dev[i].msg;
      sta["data"] = dev[i].cursta;
      String output;
      sta.printTo(output);
      body += output;
      if (i < sizeof(dev) / sizeof(dev[0]) - 1)body += ",";
    } body += "]";
    microgear.publish("/chat", String(DEVICE_NAME) + "#" + String(sender) + "#a=" + body, true);
    return;

  } else if (body.indexOf("=") > 0) {
    String strCh = splitString(body, '=', 0);
    String strVal = splitString(body, '=', 1);
    if (isValidNumber(strCh)) {
      int intCh = strCh.toInt();
      if (intCh >= (sizeof(dev) / sizeof(dev[0])))
        return;
      if (isValidNumber(strVal)) {
        int intVal = strVal.toInt();
        dev[intCh].cursta_tmp = intVal;
        if (!fading) {
          dev[intCh].cursta = intVal;


#ifndef ARDUINO_AS_IO
          setPinsValue(intCh);
#endif
        }

        Serial.println(body);
        //microgear.chat(bsender, String(wifiManager.mgear.Alias) + "#" + String(sender) + "#" + body);
#ifdef WiFiManager_h
        microgear.publish("/chat", String(wifiManager.mgear.Alias) + "#" + String(sender) + "#" + body, true);
#else
        microgear.publish("/chat", String(mgear.Alias) + "#" + String(sender) + "#" + body, true);
#endif
        return;
      } else if (strVal.equals("!")) {
        dev[intCh].cursta_tmp = map(dev[intCh].cursta, 0, 255, 255, 0);

        if (!fading) {
          dev[intCh].cursta = dev[intCh].cursta_tmp;

#ifndef ARDUINO_AS_IO
          setPinsValue(intCh);
#endif
        }
        Serial.print(intCh); Serial.print("="); Serial.println(dev[intCh].cursta);
        //microgear.chat(bsender, String(wifiManager.mgear.Alias) + "#" + String(sender) + "#" + String(intCh) + "=" + String(dev[intCh].cursta));
#ifdef WiFiManager_h
        microgear.publish("/chat", String(wifiManager.mgear.Alias) + "#" + String(sender) + "#" + String(intCh) + "=" + String(dev[intCh].cursta), true);
#else
        microgear.publish("/chat", String(mgear.Alias) + "#" + String(sender) + "#" + String(intCh) + "=" + String(dev[intCh].cursta), true);
#endif
        return;
      }
    }
  } else if (body.indexOf(">") > 0) {
    String strVal = splitString(body, '>', 0);
    String strCh = splitString(body, '>', 1);
    int iob[sizeof(dev) / sizeof(dev[0])];
    int count = 0;
    for (int i = 0; i < sizeof(iob); i++) {
      count++;
      String ch = splitString(strCh, ',', i);
      if (ch.equals("")) break;
      if (strVal.equals("!")) {
        dev[ch.toInt()].cursta_tmp = map(dev[ch.toInt()].cursta, 0, 255, 255, 0);
      }
      else {
        dev[ch.toInt()].cursta_tmp = strVal.toInt();
      }
      if (!fading) {
#ifndef ARDUINO_AS_IO
        setPinsValue(i);
#endif
      }
    }
    String bbody = "a=";
    for (int i = 0; i < sizeof(dev) / sizeof(dev[0]); i++) {
      bbody += dev[i].cursta;
      if (i < sizeof(dev) / sizeof(dev[0]) - 1)bbody += ",";
    }
    //microgear.chat(bsender, String(wifiManager.mgear.Alias) + "#" + String(sender) + "#" + bbody);
#ifdef WiFiManager_h
    microgear.publish("/chat", String(wifiManager.mgear.Alias) + "#" + String(sender) + "#" + bbody, true);
#else
    microgear.publish("/chat", String(mgear.Alias) + "#" + String(sender) + "#" + bbody, true);
#endif
    Serial.println(body);
    return;
  } else {
    inputString = body;
    processCommand(sender);
  }
}

void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  commander(msg, msglen);
}

void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  commander(msg, msglen);
}

/* When a microgear is connected, do this */
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  String s = char2String(msg, msglen);
  Serial.println("online...");

#ifndef WiFiManager_h
  microgear.setAlias(mgear.Alias);
#else
  microgear.setAlias(wifiManager.mgear.Alias);
#endif
}

uint8_t socketNum;
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  String sMsg = "";
  switch (type) {
    case WStype_DISCONNECTED:
      // Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        //IPAddress ip = webSocket.remoteIP(num);
        //Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        //webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      for (int i = 0; i < length; i++)
        sMsg += (char)payload[i];
#if defined(ARDUINO_ESP8266_NODEMCU) || defined(ESP32)
      inputString = sMsg;
      stringComplete = true;
      socketNum = num;

      Serial.println("Socket.receive(" + sMsg + ")");
#else
      Serial.println("Socket.receive(" + sMsg + ")");
#endif
      // send message to client
      // webSocket.sendTXT(num, "message here");

      // send data to all connected clients
      // webSocket.broadcastTXT("message here");
      break;
    case WStype_BIN:;
      //Serial.printf("[%u] get binary length: %u\n", num, length);
      //hexdump(payload, length);

      // send message to client
      // webSocket.sendBIN(num, payload, length);
      break;
  }

}

//-----start of button function-----------------
void press(int sender) {
  Serial.println("Press pin " + String(sender));
}
void release(int sender) {
  Serial.println("release pin " + String(sender));

}
void click(int sender) {
  Serial.println("Click pin " + String(sender));
}
void doubleClick(int sender) {
  Serial.println("Double Click pin " + String(sender));
}
void hold(int sender) {
  Serial.println("Hold pin " + String(sender));

}
void longHold(int sender) {
  Serial.println("Long Hold pin " + String(sender));
  //reset saved settings
  //wifiManager.emptyGear(); //clear gear config
#ifdef WiFiManager_h
  wifiManager.resetSettings();
#else
  clearSetting = true;
#endif
  Serial.println(F("Setting has been cleared!\n restart..."));
  // tone(D2, 494, 20);
}
//-----------end of button function-------------

//------start of Task Handle Declaration--------
//void loop is Task 0
TaskHandle_t commanderTask;   // Task 1
TaskHandle_t actionListTask;  // Task 2
TaskHandle_t buttonTask;      // Task 3
TaskHandle_t MQTTNetpieTask;  // Task 4
TaskHandle_t fadeControlTask; // Task 5

TaskHandle_t envSensorTask;   // Task 6 for System enviroment sensor (ambient, motion, temperature, etc.)
//-------end of Task Handle Declaration---------


void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
#ifdef ESP32
  hSerial2.begin(SERIAL_BAUD_RATE);
#endif

  WMProp.begin("WiFiManager", false);
  ssid = WMProp.getString("ssid", "");
  password = WMProp.getString("password", "");

  WMProp.getString("appid", "").toCharArray(mgear.AppID , 32);
  WMProp.getString("key", "").toCharArray(mgear.Key , 32);
  WMProp.getString("secret", "").toCharArray(mgear.Secret , 32);
  WMProp.getString("alias", "").toCharArray(mgear.Alias , 32);


  WMProp.end();

  Serial.println("Configuration Readed!");
  Serial.println("ssid:" + String(ssid));
  Serial.println("password:" + String(password));

  Serial.println("appid:" + String(mgear.AppID));
  Serial.println("key:" + String(mgear.Key));
  Serial.println("secret:" + String(mgear.Secret));
  Serial.println("alias:" + String(mgear.Alias));
  Serial.println("------------------------");
#ifndef ARDUINO_AS_IO
  for (int i = 0, cindex = 0; i < sizeof(dev) / sizeof(dev[0]); i++) {
    dev[i].id = cindex;
    for (byte j = 0; j < getAvailablePins(i); j++) {
#ifdef ESP8266
      pinMode(dev[i].pins[j], OUTPUT);
#elif defined(ESP32)

      //pinMode(dev[i].pins[j], OUTPUT);
      // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
      ledcSetup(cindex, pwmFrequency, pwmResolution); // 5 kHz PWM, 10-bit resolution
      // Initialize channels
      // channels 0-15
      ledcAttachPin(dev[i].pins[j], (cindex));
      cindex += 1;
      Serial.println("ledcSetup(" + String(dev[i].pins[j]) + "," + String(cindex) + ")");

#endif
    }
  }
#endif
  //set callback function for Button

  systemResetButton.eventPress((void*)press);
  systemResetButton.eventRelease((void*)release);
  systemResetButton.eventClick((void*)click);
  systemResetButton.eventDoubleClick((void*)doubleClick);
  systemResetButton.eventHold((void*)hold);
  systemResetButton.eventLongHold((void*)longHold);

  //systemResetButton.setTouchThreadHold(0);
  /*set time for event with milliseconds
    systemResetButton.setDoubleClickTime(250);
    systemResetButton.setHoldTime(2000);
    systemResetButton.setLongHoldTime(5000);
  */

  


  Serial.println(F("Starting..."));
  //pinMode(CLEAR_SETTING_TRICKER, INPUT_PULLUP);

#ifdef WiFiManager_h
  wifiManager.init();
#else
  //--------------wifi config------------......

  WiFi.onEvent(WiFiEvent);
  //Serial.println("Set Hostname...");

  if (String(ssid).equals("")) {
    WiFi.softAPConfig(IPAddress(10, 0, 0, 1), IPAddress(10, 0, 0, 1), IPAddress(255, 255, 255, 0));
    delay(1);
    WiFi.softAP(DEVICE_NAME);

    canRungear = false;
  } else {
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("Wait for WiFi connect.");
    while (WiFi.status() != WL_CONNECTED);
    canRungear = true;
  }

#endif

  // tone(D2, 440, 20);
  Serial.println(F("set microgear eepromoffset"));
  microgear.setEEPROMOffset(101);
#ifdef WiFiManager_h
  wifiManager.initGear();
  //disable WiFi debug.
  wifiManager.setDebugOutput(false);

  //use for save gear param to EEPROM
  //wifiManager.putsGear(myGear);

  //set custom ip for portal
  Serial.print(F("setup WiFi...\nif have not last connection, pending to AP Mode with SSDI:"));
  Serial.print(String(DEVICE_NAME)); Serial.println(F(" and no password"));
  wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 0, 1), IPAddress(10, 0, 0, 1), IPAddress(255, 255, 255, 0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  from DEVICE_NAME
  //and goes into a blocking loop awaiting configuration

  wifiManager.autoConnect(DEVICE_NAME);
#else

#endif

#ifdef ESP8266
  WiFi.hostname(DEVICE_NAME);
#else
  WiFi.setHostname(DEVICE_NAME);
#endif
  MDNS.begin(DEVICE_NAME);
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  Serial.print(F("WiFi Connected to ")); Serial.println(WiFi.SSID());
  Serial.print(F("IPAdress:")); Serial.println(WiFi.localIP());

  configTime(3600 * 7, daysavetime * 3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");

  /* Add Event listeners */
  Serial.println(F("Initialize Microgear..."));

  microgear.on(MESSAGE, onMsghandler);
  microgear.on(PRESENT, onFoundgear);
  microgear.on(ABSENT, onLostgear);
  microgear.on(CONNECTED, onConnected);
#ifdef WiFiManager_h
  if (wifiManager.mgear.AppID != "" && !wifiManager.mgear.Key != "" && !wifiManager.mgear.Secret != "" ) {
    canRungear = true;
    Serial.println(F("start Microgear..."));
    if (wifiManager.mgear.Alias == "")
      microgear.init(wifiManager.mgear.Key, wifiManager.mgear.Secret, DEVICE_NAME);
    /* Initial with KEY, SECRET and also set the ALIAS here */
    else microgear.init(wifiManager.mgear.Key, wifiManager.mgear.Secret, wifiManager.mgear.Alias);
    Serial.println("AppID:" + String(wifiManager.mgear.AppID) + "\nKey:" + String(wifiManager.mgear.Key) + "\nSecret:" + String(wifiManager.mgear.Secret) + "\nAlias:" + String(wifiManager.mgear.Alias));
    Serial.println(F("Connecting Microgear to NETPIE..."));
    /* connect to NETPIE to a specific APPID */

    while (systemResetButton.isPress())delay(10);
    if (clearSetting == true) {
      clearWiFiSetting();
#if defined(ESP8266)
      ESP.reset();
#else
      ESP.restart();
#endif
    }
    microgear.connect(wifiManager.mgear.AppID);
    microgear.subscribe("/chat");
  }
#else
  if (!String(mgear.AppID).equals("") && !String(mgear.Key).equals("") && !String(mgear.Secret).equals("") && canRungear == true) {
    Serial.println(F("---start Microgear..."));
    if (mgear.Alias == "")
      microgear.init(mgear.Key, mgear.Secret, DEVICE_NAME);
    /* Initial with KEY, SECRET and also set the ALIAS here */
    else microgear.init(mgear.Key, mgear.Secret, mgear.Alias);
    Serial.println("AppID:" + String(mgear.AppID) + "\nKey:" + String(mgear.Key) + "\nSecret:" + String(mgear.Secret) + "\nAlias:" + String(mgear.Alias));
    Serial.println(F("Connecting Microgear to NETPIE..."));
    /* connect to NETPIE to a specific APPID */

    while (systemResetButton.isPress())delay(10);
    if (clearSetting == true) {
      clearWiFiSetting();
#if defined(ESP8266)
      ESP.reset();
#else
      ESP.restart();
#endif
    }
    microgear.connect(mgear.AppID);
    microgear.subscribe("/chat");
  }
#endif
  else {
    canRungear = false;
    Serial.println(F("Microgear was silent! because anything is wrong!\nPlease press reset to reboot"));
  }
  initHandleds();
  webServer.begin();
  Serial.print(F("HTTP PORT:"));
  Serial.println(String(HTTP_PORT));

  //  socketServer.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.print(F("TCP PORT:"));
  Serial.println(String(TCP_PORT));

  Serial.println("Reading Check Times");
  readTimes();
  Serial.println(F("done"));



  //2.serial and message task
#ifdef ESP32
  xTaskCreate(&Button_Task, "Button_Task", 2048, NULL, 2, &buttonTask);
  xTaskCreate(&Commander_Task, "Commander_Task", 2048, NULL, 1, &commanderTask);
  xTaskCreate(&ActionList_Task, "ActionList_Task", 2048, NULL, 1, &actionListTask);
  xTaskCreate(&MQTT_NETPIE_TASK, "mqttnetpietask", 10240, NULL, 3, &MQTTNetpieTask);
  xTaskCreate(&FadeControl_TASK, "fadeControltask", 2048, NULL, 1, &fadeControlTask);

  //xTaskCreate(&EnvSensor_Task, "envSensorTask", 2048, NULL, 1 , &envSensorTask);

  Serial.setDebugOutput(0);
#endif

}
void Button_Task(void *p) { //loop2
  while (1) {
    systemResetButton.handleButton();
    delay(1);
  }
}
void Commander_Task(void *p) { //loop3
  while (1) {
    //Serial.println("Hello");
    serialEvent();
    //serial2Event();
    if (stringComplete) {
      processCommand();
      //Serial.println(inputString);
      //y79l;socketClient.println(inputString);
      inputString = "";
      stringComplete = false;
    }
  }
}
void MQTT_NETPIE_TASK(void *p) { //loop4
  while (1) {
    //netpie task
    //if allow to use microgear..
    unsigned long currentMillis = millis();
    if (canRungear) {
      /* To check if the microgear is still connected */
      if (microgear.connected()) {
        microgear.loop();
        nextTime = 0;
      } else {
        if (currentMillis - pMillis >= (1000 * 60)*nextTime) {
          //if connection lost then reconnect.
          pMillis = currentMillis;
          Serial.println(F("reconnecting...."));
          microgear.resetToken();
#ifndef WiFiManager_h
          microgear.connect(mgear.AppID);
#else
          microgear.connect(wifiManager.mgear.AppID);
#endif
          microgear.subscribe("/chat");
          if (!microgear.connected()) {
            nextTime += 1; //nextTime in Minutes
            if (nextTime > 5) {
              nextTime = 5;
            }
            Serial.print(F("recheck connection in next "));
            Serial.print(nextTime);
            Serial.println(F(" minutes"));
          }
        }
      }

    }
    delay(20);
  }
}
void FadeControl_TASK(void *p) { //loop5
  while (1) {
    //----------------
    //Task ควบคุมหลอดไฟ......
    for (int i = 0; i < sizeof(dev) / sizeof(dev[0]); i++) {
      if (fading) {
        if (dev[i].cursta < dev[i].cursta_tmp) {
          dev[i].cursta += 1;
        } else if (dev[i].cursta > dev[i].cursta_tmp) {
          dev[i].cursta -= 1;
        }
      } else {
        dev[i].cursta = dev[i].cursta_tmp;
      }
      setPinsValue(i);
      //Serial.print(String(i)+":("+String(dev[i].cursta)+","+String(dev[i].cursta_tmp)+")");
      //if(i < sizeof(dev) / sizeof(dev[0])-1)Serial.print(",");
    }
    //Serial.println();
    delay(fadingInterval);
    //Serial.println("fade running...");
  }
}
void loop() { //loop1 (main loop)
  //still running webserver
  webServer.handleClient();
  if (clearSetting == true) {
    clearWiFiSetting();
#if defined(ESP8266)
    ESP.reset();
#else
    ESP.restart();
#endif
  }
  //update running status to arduino
  //  if (currentMillis - pRunning >= (sta_running_duration / 2)) {
  //    Serial.println("RUN:1");
  //    pRunning = currentMillis;
  //  }

  //still running websocket
  webSocket.loop();
}

#endif
