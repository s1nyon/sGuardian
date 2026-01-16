#include <Arduino.h>


void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);    //  LED_BUILTIN 是默认的板上的 LED 灯，gpio 默认引脚是 13
  Serial.begin(9600);
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("blink\n");
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}