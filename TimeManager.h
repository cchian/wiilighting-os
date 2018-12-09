#ifndef TIME_MANAGER
#define TIME_MANAGER
#include <TimeLib.h>
#include <LinkedList.h>


#include "Properties.h"
extern void setPinsValue(int i); //declared in MyFunction.h line 66
/*
  void sendMessageToNode(Action * ac) {
  String nName = ac->nodeNames;
  nName.replace("{", "");
  nName.replace("}", "");
  int iob[sizeof(dev) / sizeof(dev[0])];
  for (int i = 0; ; i++) {
    String ch = splitString(nName, ',', i);
    if (ch.equals(""))break;
    if (ch.equals(dev[i].nodeName)) {
      dev[i].cursta = ac->lightValue;
    }
  }
  String ss = String(DEVICE_NAME) + "#" + ac->nodeNames + "#" + ac->lightValue;
  nodeSerial.println(ss);
  }
*/

void do_Action(Action * ac) {
  if (ac->lightValue.startsWith("f") || ac->lightValue.startsWith("F")) {
    String strValue = ac->lightValue;
    strValue.replace("f", "");
    strValue.replace("F", "");
    dev[ac->nodeNames.toInt()].cursta_tmp = strValue.toInt();
    fading=true;
    vTaskResume(fadeControlTask);
    //...................
  } else {
    fading=false;
    vTaskSuspend(fadeControlTask);
    dev[ac->nodeNames.toInt()].cursta_tmp = ac->lightValue.toInt();
    dev[ac->nodeNames.toInt()].cursta=dev[ac->nodeNames.toInt()].cursta_tmp;
    setPinsValue(ac->nodeNames.toInt());
    //.....
  }
  Serial.println(String(ac->nodeNames.toInt())+"="+String(dev[ac->nodeNames.toInt()].cursta_tmp));
}

void ActionList_Task(void *p) {
  struct tm t;
  getLocalTime(&t, 5000);
  int ps = t.tm_sec - 1;
  while (1) {
    if (ps == t.tm_sec) {
      getLocalTime(&t, 5000);
      continue;
    }
    ps = t.tm_sec;
    Serial.println("CTIME -> " +
                   String(t.tm_year + 1900) + "/" +
                   String(t.tm_mon + 1) + "/" +
                   String(t.tm_mday) + " " +
                   ((t.tm_hour < 10) ? "0" : "") +
                   String(t.tm_hour) + ":" +
                   ((t.tm_min < 10) ? "0" : "") +
                   String(t.tm_min) + ":" +
                   ((t.tm_sec < 10) ? "0" : "") +
                   String(t.tm_sec)
                  );
                  
    if (actionList.size() > 0) {
      
      for (int i = 0; i < actionList.size(); i++) {
        Action *ac = actionList.get(i);
        //Serial.println(String(dayOfWeek(ac->t)) + " == " + String(t.tm_wday+1));
        if (year(ac->t) >= t.tm_year + 1900) {
          if (
            year(ac->t) == (t.tm_year + 1900) &&
            month(ac->t) == (t.tm_mon + 1) &&
            day(ac->t) == (t.tm_mday) &&
            hour(ac->t) == (t.tm_hour) &&
            minute(ac->t) == (t.tm_min) &&
            second(ac->t) == (t.tm_sec)
          ) {
            do_Action(ac);
            actionList.remove(i);
            i--;
            if (actionList.size() == 0)
              clearTimes();
          }
        } else if (year(ac->t) == 1972) {
          //yeayly ประจำปี
          if (
            month(ac->t) == (t.tm_mon + 1) &&
            day(ac->t) == (t.tm_mday) &&
            hour(ac->t) == (t.tm_hour) &&
            minute(ac->t) == (t.tm_min) &&
            second(ac->t) == (t.tm_sec)
          ) {
            do_Action(ac);
          }
        } else if (year(ac->t) == 1971) {
          //monthly ประจำเดือน (รายเดือน)
          if (
            day(ac->t) == (t.tm_mday) &&
            hour(ac->t) == (t.tm_hour) &&
            minute(ac->t) == (t.tm_min) &&
            second(ac->t) == (t.tm_sec)
          ) {
            do_Action(ac);
          }
        } else if (year(ac->t) == 1970) {
          //daily ประจำวัน (รายวัน)
          if (
            hour(ac->t) == (t.tm_hour) &&
            minute(ac->t) == (t.tm_min) &&
            second(ac->t) == (t.tm_sec)
          ) {
            do_Action(ac);
          }
        } else if (year(ac->t) == 1973) {
          //houry ประจำชม. (ราย ชม.)
          if (
            minute(ac->t) == (t.tm_min) &&
            second(ac->t) == (t.tm_sec)
          ) {
            do_Action(ac);
          }
        } else if (year(ac->t) == 1974) {
          //weekly ประจำสัปดาห์ (เลือกวันในสัปดาห์)

          if (
            dayOfWeek(ac->t) == (t.tm_wday + 1) &&
            hour(ac->t) == (t.tm_hour) &&
            minute(ac->t) == (t.tm_min) &&
            second(ac->t) == (t.tm_sec)
          ) {
            do_Action(ac);// send message command to node device (this  line was define for smart lighting device for esp8266)
          }
        }
      }
    }
    delay(1000);
    getLocalTime(&t, 5000);
  }
}
#endif
