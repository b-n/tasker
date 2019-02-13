#include "IOExpander.h"

void IOExpander::MCPwrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MCP23017_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

uint8_t IOExpander::MCPread(uint8_t reg) {
  Wire.beginTransmission(MCP23017_ADDR);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(MCP23017_ADDR, 1);
  return Wire.read();
}

void IOExpander::clearInterrupt() {
  readInterrupt();
}

void IOExpander::writePins(uint8_t val) {
  MCPwrite(GPIOB, val);
}

uint8_t IOExpander::readPins() {
  return MCPread(GPIOA);
}

uint8_t IOExpander::readInterrupt() {
  return MCPread(INTCAPA);
}

void IOExpander::ISR() {
  interrupted = true;
  interruptTime = millis();
}

void IOExpander::setup() {
  //set GPIOA inputs with pullups
  MCPwrite(IODIRA, 0xFF);
  MCPwrite(PULLUPA, 0xFF);

  //set GPIOB output and default values
  MCPwrite(IODIRB, 0);
  MCPwrite(GPIOB, 0);

  //setup interrupts for all pins
  MCPwrite(IOCONA, 0x40); //enable PortA, mirroring, int is active-low
  MCPwrite(INTCONA, 0); //compare to previous value
  MCPwrite(GPINTENA, 0xFF); //enable interrupt
}

void IOExpander::setInterrupt(uint8_t pin, std::function<void(uint8_t newState, uint8_t oldState)> changeCallback) {
  interruptPin = pin;
  onChange = changeCallback;
  pinMode(pin, INPUT_PULLUP);
  clearInterrupt();
}
 
 
void IOExpander::loop() {
  if (interruptPin == NULL) return;
  
  if (interrupted && millis() - interruptTime > 20) {
    uint8_t  newState = readInterrupt() ^ 0xFF;
    interrupted = false;

    if (newState != state) {
      this->onChange(newState, state);
      state = newState;
    }
  }

  //sometimes the interrupt wasn't clearing, so attempt to fix that
  if (!interrupted && digitalRead(interruptPin) == 0) {
    clearInterrupt();
  }
}
