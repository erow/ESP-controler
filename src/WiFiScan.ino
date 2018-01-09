/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include <Arduino.h>
#include "ESP8266WiFi.h"
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "FS.h"

int led_pin=LED_BUILTIN;

StaticJsonBuffer<200> jsonBuffer;
std::unique_ptr<char[]> buf;
JsonVariant root = jsonBuffer.createObject();
bool loadConfig() {
  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    Serial.println("Failed to open config file");
    return false;
  }
  int size=file.size();
  buf=std::unique_ptr<char[]> (new char[size]);
  file.readBytes(buf.get(),size);
  root = jsonBuffer.parseObject(buf.get());

  root.prettyPrintTo(Serial);
  Serial.println();
}

bool saveConfig() {
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  root.printTo(configFile);
  return true;
}

void initOTA(){
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
}
void setup() {
  Serial.begin(115200);
  Serial.printf("start esp\n");
  Serial.println("Mounting FS...");
  
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  loadConfig();
  Serial.printf_P("suc:%d,ssid:%d\n",root.success(),
  root.asObject().containsKey("ssid"));
  if(root.success()&&
  root.asObject().containsKey("ssid")){
    //JsonObject& conf=root.asObject();
    WiFi.begin(root["ssid"].asString(),root["key"].asString());
    Serial.print("Connecting");
    int c=0;
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(500);
      c++;
      if(c>200)
      {
        root=jsonBuffer.createObject();
        ESP.reset();
      }
    }
    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());

  }else{
    WiFi.softAP("ESP","12345678");
    Serial.println("create ap esp-12345678");
  }
  initOTA();
}

void loop() {
  ArduinoOTA.handle();

  //digitalWrite(led_pin,!digitalRead(led_pin));
  //ESP.deepSleep(5e4);
}
