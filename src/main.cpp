#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Preferences.h>

#define WDT_TIMEOUT 5  
#define LED_PIN 2    //Breathing LED pin
#define BUTTON_PIN 0    //Mode switch button pin

Preferences preferences;
bool lastButtonState=HIGH;    //Record previous button state
int currentMode=0;     
 
void setup(){
  Serial.begin(115200);
  Serial.println("\n--- [Day 3] NVS Initialization ---");
  preferences.begin("my-app",false);
  Serial.println("[NVS] Preferences library initialized");
  currentMode=preferences.getInt("let_mode",0);
  Serial.printf("[NVS] Recovered LED Mode: %d\n", currentMode);
  esp_task_wdt_init(WDT_TIMEOUT,true);  //Initialize WDT
  //Add current task to WDT
  esp_task_wdt_add(NULL);
  Serial.println("Watchdog activated.");
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  pinMode(LED_PIN,OUTPUT);
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
  bool buttonState=digitalRead(BUTTON_PIN);
  if (buttonState==LOW&&lastButtonState==HIGH)
  {
    delay(50);
    if (buttonState==LOW)
    {
      currentMode=(currentMode+1)%3;   //Cycle through 0,1,2
      Serial.printf("[MODE] Changed to: %d\n", currentMode);
      preferences.putInt("led_mode",currentMode);   //Write to NVS immediately 
       Serial.println("[NVS] State saved safely to flash.");
    }
  }
  lastButtonState==buttonState;
  
}

void executeLEDMode(int mode)
{
  static uint32_t lastUpState=0;
  static int brightness=0;
  static int fadeAmount=5;
  if (mode==0)
  {
    analogWrite(LED_PIN,0);
  }else if (mode==1)
  {
    if (millis()-lastUpState>30)
    {
       analogWrite(LED_PIN,brightness);
       brightness=brightness+fadeAmount;
       if (brightness<=0||brightness>=255)
       {
        fadeAmount=-fadeAmount;
       }
       
    }
    lastUpState=millis();
  }else if (mode==2)
  {
    if (millis()-lastUpState>100)
    {
      brightness=(brightness==0)? 255:0;
      analogWrite(LED_PIN,brightness);
      lastUpState=millis();
    }
  }
}