#include <Arduino.h>
#include <esp_task_wdt.h>

#define WDT_TIMEOUT 5  

void setup(){
  Serial.begin(115200);
  Serial.println("\n--- [Day 1] WDT Init ---");
  esp_task_wdt_init(WDT_TIMEOUT,true);  //Initialize WDT
  //Add current task to WDT
  esp_task_wdt_add(NULL);
  Serial.println("Watchdog activated.");
}

void loop(){
  //Periodically feed the dog
  esp_task_wdt_reset();
  delay(100);
}