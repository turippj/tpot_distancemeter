#include <TURIPserver.h>
#include <TURIPserverSPI.h>
#include <TURIPshell.h>
#include <EEPROM.h>

#include <Wire.h>
#include "SparkFun_VL53L1X_Arduino_Library.h"

#define TURIP_MODEL 0x100A
#define TURIP_SEREAL 0x0

VL53L1X distanceSensor;

TURIPport* PortDistance;
TURIPport* PortSerialNum;

unsigned int Distance; 

void setup(){
  Serial.begin(9600);
  Wire.begin();
  TURIPserver.begin(TURIP_MODEL, TURIP_SEREAL);
  TURIPserverSPI.begin();
  PortDistance = TURIPserver.newPort(1);
  PortSerialNum = TURIPserver.getPort(TURIP_PORT_SERIAL);
  PortSerialNum->writeUint32(getSerialNum());
  PortSerialNum->postReceiveFunc = updateSerialNum;
  
  Distance = 0;
  PortDistance->writeUint16(Distance);
  distanceSensor.begin();
}

void loop(){
  static unsigned long ms = 0;
  if(ms + 5 < millis()){
    ms = millis();
    if(distanceSensor.newDataReady()){
      Distance = distanceSensor.getDistance();
      PortDistance->writeUint16(Distance);
    }
  }
  TURIPserverSPI.update();
  serialEvent();
}

int updateSerialNum(){
  uint32_t serialNum = PortSerialNum->readUint32();
  EEPROM.update(0, (uint8_t)serialNum);
  EEPROM.update(1, (uint8_t)(serialNum >> 8));
  EEPROM.update(2, (uint8_t)(serialNum >> 16));
  EEPROM.update(3, (uint8_t)(serialNum >> 24));
}

uint32_t getSerialNum(){
  return EEPROM[0] + (EEPROM[1] << 8) + (EEPROM[2] << 16) + (EEPROM[3] << 24);
}

void serialEvent(){
  static String strBuf;
  while(Serial.available()){
    char c = Serial.read();
    if(c == 0x0a){  // 0x0a: LF
      strBuf.trim();
      if(strBuf.length() > 0){
        char response[128];
        char request[64];
        strBuf.toCharArray(request, 64);
        TURIPshell(request, response, 127);
        Serial.println(response);
      }
      strBuf = "";
    }else{
      strBuf += c;
    }
  }
}
