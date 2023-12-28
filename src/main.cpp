#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <GyverMotor2.h>
#include <GParser.h>

// Определяем название и пароль точки доступа
const char* __SSID = "HORIZONE";
const char* __PSWD = "123456789";
const uint32_t __PORT = 49152;

const int8_t BRIDGE_MOTOR_PIN_DIR = 13;
const int8_t BRIDGE_MOTOR_PIN_PWM = 12;

const int8_t TROLLEY_MOTOR_PIN_DIR = 2;
const int8_t TROLLEY_MOTOR_PIN_PWM = 0;

const int8_t WINCH_MOTOR_PIN_DIR = 4;
const int8_t WINCH_MOTOR_PIN_PWM = 5;

const char* HELLO_MSG = "_A_BRIDGE";
//const char* HELLO_MSG = "_B_SPREADER";
//const char* HELLO_MSG = "_C_CRANES";
//const char* HELLO_MSG = "_D_LIFTS";
const uint8_t BUFFER_SIZE = 32;

static uint32_t tmr_ping = millis();
static uint32_t tmr_ping_interval = 50; //1354;

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

  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.localIP());
  
  //ping();  

  UDP.beginPacket(WiFi.gatewayIP(), __PORT);
  UDP.printf(HELLO_MSG);
  UDP.endPacket();

  UDP.begin(__PORT);

  MOT_Bridge.setMinDuty(220); // мин. ШИМ
  MOT_Bridge.reverse(1);     // реверс
  MOT_Bridge.setDeadtime(5); // deadtime
  MOT_Bridge.smoothMode(1); 

  MOT_Trolley.setMinDuty(220); // мин. ШИМ
  MOT_Trolley.reverse(1);     // реверс
  MOT_Trolley.setDeadtime(5); // deadtime
  MOT_Trolley.smoothMode(1); 
  

  MOT_Winch.setMinDuty(220); // мин. ШИМ
  MOT_Winch.reverse(1);     // реверс
  MOT_Winch.setDeadtime(5); // deadtime
  MOT_Winch.smoothMode(1); 
  
}

void loop()
{
  MOT_Bridge.tick();
  MOT_Trolley.tick();
  MOT_Winch.tick();

  if (WiFi.status() != WL_CONNECTED)
  {
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      WiFi.mode(WIFI_STA);
      WiFi.begin(__SSID, __PSWD);
      delay(250);     
    }
  }

  /*if (millis() - tmr_ping > tmr_ping_interval)
  {
    tmr_ping = millis();
    ping();
  }*/

  int packetSize = UDP.parsePacket();
  if (packetSize)
  {
    //PACKET 0,1,-255,-255,-255;
    Serial.print("Received packet! Size: ");
    Serial.println(packetSize);
    Serial.print("Packet: ");
    char packet[BUFFER_SIZE];
    int len = UDP.read(packet, BUFFER_SIZE);
    Serial.println(packet);
    if(len > 0)
    {
      packet[len] = '\0';      
      GParser data(packet);
      int ints[data.amount()];
      data.parseInts(ints);

      switch (ints[0])
      {
      case 0:
        if (ints[1])
        {
          //delay(3);
              
          MOT_Bridge.setSpeed(ints[3]);
          MOT_Trolley.setSpeed(ints[4]);
          MOT_Winch.setSpeed(ints[5]);

          MOT_Bridge.tick();
          MOT_Trolley.tick();
          MOT_Winch.tick();
          
        }
        //else
          //stopAllMotors();
        break;

      default:
        //stopAllMotors();
        break;
      }
    }
  }
  /*else
  {
    stopAllMotors();
  }*/
}

/*void stopAllMotors()
{
  //delay(2.5);

  MOT_Bridge.setSpeed(0);
  MOT_Trolley.setSpeed(0);
  MOT_Winch.setSpeed(0);
  
  MOT_Bridge.tick();
  MOT_Trolley.tick();
  MOT_Winch.tick();

  

}

void ping()
{
  UDP.beginPacket(WiFi.gatewayIP(), __PORT);
  UDP.printf(HELLO_MSG);
  UDP.endPacket();
}*/