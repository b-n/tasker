
#include <Arduino.h>
#include <ArduinoJson.h>
#include "WifiManager.h"
#include "IoT.h"
#include "IOExpander.h"

#include "WifiSettings.h"
#include "IoTSettings.h"

#define INTERRUPT_PIN D2
#define WIRE_SDA D4
#define WIRE_SCL D5

WifiManager wifiManager(STA_SSID, STA_PSK);
IoT awsThing(THING_NAME, aws_endpoint, aws_key, aws_secret, aws_region);
IOExpander expander = IOExpander();

int currentProjectId = 0;
int targetProjectId = currentProjectId;

long lastConfirmed = 0;
bool changeConfirmed = true;
long lastRequested = 0;
bool changeRequested = false;
bool changeNotified = false;

void AWSCallback(char* topic, byte* payload, unsigned int length) {
  char buf[MQTT_MAX_PACKET_SIZE];
  strncpy(buf, (const char *)payload, length);
  buf[length] = '\0';

  if (strcmp(topic,"$aws/things/tasker/shadow/update/accepted") == 0 || strcmp(topic,"$aws/things/tasker/shadow/get/accepted") == 0) {
    lastConfirmed = millis();
    changeConfirmed = true;
    changeNotified = false;
    DynamicJsonBuffer jsonBuffer(512);
    JsonObject& root = jsonBuffer.parseObject(buf);
    int desiredProjectId = root["state"]["desired"]["project"];
    updateProject(desiredProjectId);
  }
}

void updateAWS(uint8_t projectId) {
  const size_t capacity = 3*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2);
  DynamicJsonBuffer jsonBuffer(capacity);
  
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& state = root.createNestedObject("state");
  JsonObject& desired = state.createNestedObject("desired");
  desired["project"] = projectId;
  JsonObject& reported = state.createNestedObject("reported");
  reported["project"] = projectId;

  char buffer[512];
  root.printTo(buffer, root.measureLength()+1);

  awsThing.sendState(buffer);
}

void updateProject(int projectId) {
  updateLEDs(projectId);
  if (projectId != currentProjectId) {
    currentProjectId = projectId;
    targetProjectId = projectId;
    changeConfirmed = false;
    updateAWS(currentProjectId);
  }
}

void updateLEDs(uint8_t value) {
  expander.writePins(value);
}

void connectedCallback() {
  //doing this call too quickly after connecting means the message might not show up
  delay(100);
  awsThing.getState();
}

void expanderISR() {
  expander.ISR();
}

void onButtonChange(uint8_t newState, uint8_t oldState) {
  // the button(s) being released
  uint8_t changed = (oldState ^ newState) & oldState;
  if (!changed) return;
  
  targetProjectId = targetProjectId ^ changed;
  updateLEDs(targetProjectId);
  lastRequested = millis();
  changeRequested = true;
}

void setup() {
  Wire.begin(WIRE_SDA, WIRE_SCL);

  expander.setup();
  expander.setInterrupt(INTERRUPT_PIN, onButtonChange);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), expanderISR, FALLING);

  wifi_set_sleep_type(NONE_SLEEP_T);
  wifiManager.setup();
  
  awsThing.setup();
  awsThing.setCallback(AWSCallback);
  awsThing.setConnectedCallback(connectedCallback);
}

void loop() {

  //debounce button changes before sending to AWS
  long now = millis();
  if (changeRequested && now - lastRequested > 1500) {
    changeRequested = false;
    updateProject(targetProjectId);
  }

  //flash lights when confirmed via AWS
  if (changeConfirmed && !changeNotified) {
    long elapsed = now - lastConfirmed;
    if (elapsed > 300) {
      updateLEDs(currentProjectId);
      changeNotified = true;
    } else {
      bool state = ((elapsed / 50) % 2);
      updateLEDs(state ? 0xFF : 0);
    }
  }

  // handle port expander
  expander.loop();

  // manage wifi and AWs
  wifiManager.loop();
  if (wifiManager.status() == WL_CONNECTED) {
    awsThing.loop();
  }
}
