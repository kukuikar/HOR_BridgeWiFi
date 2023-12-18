#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <GyverMotor2.h>
#include <GParser.h>

// Определяем название и пароль точки доступа
#include "WIFI_AP.h"

const int8_t BRIDGE_MOTOR_PIN_DIR = 2;
const int8_t BRIDGE_MOTOR_PIN_PWM = 3;

const int8_t TROLLEY_MOTOR_PIN_DIR = 5;
const int8_t TROLLEY_MOTOR_PIN_PWM = 6;

const int8_t WINCH_MOTOR_PIN_DIR = 7;
const int8_t WINCH_MOTOR_PIN_PWM = 1;

const char* HELLO_MSG = "_A_BRIDGE";
const int8_t BUFFER_SIZE = 32;

static uint32_t tmr_ping = millis();
static uint32_t tmr_ping_interval = 130;

GMotor2<DRIVER2WIRE> MOT_Bridge(  BRIDGE_MOTOR_PIN_DIR,   BRIDGE_MOTOR_PIN_PWM);
GMotor2<DRIVER2WIRE> MOT_Trolley( TROLLEY_MOTOR_PIN_DIR,  TROLLEY_MOTOR_PIN_PWM);
GMotor2<DRIVER2WIRE> MOT_Winch(   WINCH_MOTOR_PIN_DIR,    WINCH_MOTOR_PIN_PWM);

WiFiUDP UDP;

void stopAllMotors();
void ping();

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(__SSID, __PSWD); 
  
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(250);
  }  

  ping();

  UDP.begin(__PORT);

  MOT_Bridge.setMinDuty(70); // мин. ШИМ
  MOT_Bridge.reverse(1);     // реверс
  MOT_Bridge.setDeadtime(5); // deadtime

  MOT_Trolley.setMinDuty(70); // мин. ШИМ
  MOT_Trolley.reverse(1);     // реверс
  MOT_Trolley.setDeadtime(5); // deadtime

  MOT_Winch.setMinDuty(70); // мин. ШИМ
  MOT_Winch.reverse(1);     // реверс
  MOT_Winch.setDeadtime(5); // deadtime
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(250);
    }
  }

  if (millis() - tmr_ping > tmr_ping_interval)
  {
    tmr_ping = millis();
    ping();
  }

  if (UDP.parsePacket())
  {
    //PACKET 0,1,-255,-255,-255;
    Serial.print("Received packet! Size: ");
    Serial.println(UDP.parsePacket());
    Serial.print("Packet: ");
    char buffer[BUFFER_SIZE];
    if (UDP.read(buffer, BUFFER_SIZE))
    {
      Serial.println(buffer);
      GParser data(buffer);
      int ints[data.amount()];
      data.parseInts(ints);

      switch (ints[0])
      {
      case 0:
        if (ints[1])
        {
          MOT_Bridge.setSpeed(ints[2]);
          MOT_Trolley.setSpeed(ints[3]);
          MOT_Winch.setSpeed(ints[4]);
        }
        else
          stopAllMotors();
        break;

      default:
        stopAllMotors();
        break;
      }
    }
  }
  else
  {
    stopAllMotors();
  }
}

void stopAllMotors()
{
  MOT_Bridge.setSpeedPerc(0);
  MOT_Trolley.setSpeedPerc(0);
  MOT_Winch.setSpeedPerc(0);
}

void ping()
{
  UDP.beginPacket(WiFi.gatewayIP(), __PORT);
  UDP.printf(HELLO_MSG);
  UDP.endPacket();
}