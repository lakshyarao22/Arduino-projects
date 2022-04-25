// Relay Main = D13
// Relay Delta = D12
// Relay Star = D14
#define BLYNK_TEMPLATE_ID "TMPLnBmN4kIB"
#define BLYNK_DEVICE_NAME "ACRemote"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial

#define APP_DEBUG


#define PRIMARY 13
#define STAR 12
#define DELTA 14
#include "BlynkEdgent.h"

int switchValue = 0;
int durationValue = 5;
String duration = "5000";

void turnOn(String duration){

  digitalWrite(PRIMARY, LOW);
  digitalWrite(STAR, LOW);
  delay(duration.toInt());
  digitalWrite(STAR, HIGH);
  digitalWrite(DELTA, LOW);
  };
void turnOff(){

  digitalWrite(PRIMARY, HIGH);
  digitalWrite(DELTA, HIGH);
  digitalWrite(STAR, HIGH);
  };

BLYNK_WRITE(V1){
  durationValue = param.asInt();
  duration = String(durationValue) + "000";
}

BLYNK_WRITE(V0){
  switchValue = param.asInt();
switch (switchValue) {
  case 1:
  turnOn(duration);
    break;
  case 0:
  turnOff();
    break;
  default:
  turnOff();
    break;
}
}

void setup()
{
  Serial.begin(115200);
  pinMode(PRIMARY, OUTPUT);
  pinMode(STAR, OUTPUT);
  pinMode(DELTA, OUTPUT);
  delay(100);

  BlynkEdgent.begin();
}

void loop() {
  BlynkEdgent.run();
}

