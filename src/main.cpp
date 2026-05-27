#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Preferences.h>

#define WDT_TIMEOUT 5  

Preferences preferences;

void setup(){
  Serial.begin(115200);
  Serial.println("\n--- [Day 3] NVS Initialization ---");
  preferences.begin("my-app",false);
  Serial.println("[NVS] Preferences library initialized");
  esp_task_wdt_init(WDT_TIMEOUT,true);  //Initialize WDT
  //Add current task to WDT
  esp_task_wdt_add(NULL);
  Serial.println("Watchdog activated.");
}

void loop(){
  //Periodically feed the dog
  esp_task_wdt_reset();
  //Trigger conditon:Listen for specific char
  if (Serial.available())
  {
    char c=Serial.read();
    if (c=='X'||c=='x')
    {
      Serial.println("\n[ERROR] Deadlock Triggered! Watchdog will bite in 5 seconds...");
      //Deliberately create system deadlock
      while (1)
      {
        
      }
    }
  }
  delay(100);
}