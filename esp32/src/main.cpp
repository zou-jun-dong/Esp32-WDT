#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#define WDT_TIMEOUT 10  
#define LED_PIN 2    //Breathing LED pin
#define BUTTON_PIN 0    //Mode switch button pin

const char* ssid = "iPhone";
const char* password = "zjd20061212";
const char* mqtt_broker = "broker.emqx.io";
const char* mqtt_topic = "esp32/device/control";

Preferences preferences;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

bool lastButtonState=HIGH;    //Record previous button state
int currentMode=0;     
 
void setup(){
  Serial.begin(115200);
  Serial.println("\n--- System Booting ---");

  preferences.begin("my-app",false);
  Serial.println("[NVS] Preferences library initialized");
  currentMode=preferences.getInt("let_mode",0);
  Serial.printf("[NVS] Recovered LED Mode: %d\n", currentMode);

  pinMode(BUTTON_PIN,INPUT_PULLUP);
  pinMode(LED_PIN,OUTPUT);

  setupWiFi();
  mqttClient.setServer(mqtt_broker,1883);
  mqttClient.setCallback(mqttCallback);

  esp_task_wdt_init(WDT_TIMEOUT,true);  //Initialize WDT
  //Add current task to WDT
  esp_task_wdt_add(NULL);
  Serial.println("[WDT] Watchdog activated.");
}

void loop(){
  //Periodically feed the dog
  esp_task_wdt_reset();
  if (!mqttClient.connected())
  {
    reconnectMQTT();
  }
  mqttClient.loop();
  
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
  executeLEDMode(currentMode);
}

void mqttCallback(char* topic,byte* playload,unsigned int length)
{
  Serial.print("\n[MQTT] Message arrived on topic: ");
  Serial.println(topic);

  String message;
  for (int i = 0; i < length; i++)
  {
    message = message + (char)playload[i];
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc,message);

  if (error)
  {
    Serial.print("[JSON] Parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  String cmd=doc["cmd"];
  if (cmd == "set_mode")
  {
    int newValue = doc["value"];
    if (newValue >= 0 && newValue <= 3)
    {
      currentMode = newValue;
      preferences.putInt("led_mode",currentMode);
      Serial.printf("[CLOUD] Mode updated to %d and safely saved to NVS.\n", currentMode);
    } else {
      Serial.println("[ERROR] Invalid mode value received.");
    }
  }
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

void setupWiFi()
{
  delay(50);
  Serial.printf("\n[WiFi] Connecting to %s",ssid);
  WiFi.begin(ssid,password);
  if (!WiFi.isConnected())
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected! IP: "+WiFi.localIP().toString());
}

void reconnectMQTT()
{
  while (!mqttClient.connected())
  {
    Serial.println("[MQTT] Attempting connection");
    String clientId = "ESP32Client-"+ String(random(0xffff),HEX);

    esp_task_wdt_reset;

    if (mqttClient.connect(clientId.c_str()))
    {
      Serial.println("Connected");
      mqttClient.subscribe(mqtt_topic);
  } else {
    Serial.print("failed,rc=");
    Serial.print(mqttClient.state());
    Serial.println(" try again in 5 seconds");
    delay(5000);
    }
  
  }
}