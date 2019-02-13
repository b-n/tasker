#ifndef IOEXPANDER_H
#define IOEXPANDER_H

#include <Arduino.h>
#include <Wire.h>

#define MCP23017_ADDR 0x20

#define IODIRA   0x00
#define PULLUPA  0x0C
#define GPIOA    0x12
#define GPINTENA 0x04
#define INTCONA  0x08
#define IOCONA   0x0A
#define INTFA    0x0E
#define INTCAPA  0x10

#define IODIRB   0x01
#define GPIOB    0x13

class IOExpander {

  public:
    IOExpander() {
       interrupted = false;
       interruptTime = 0;
    }

    void setInterrupt(uint8_t pin, std::function<void(uint8_t newState, uint8_t oldState)> changeCallback);
    void writePins(uint8_t value);
    uint8_t readPins();
    uint8_t readInterrupt();
    void ISR();
    
    void setup();
    void loop();
    
  private:
    void MCPwrite(uint8_t reg, uint8_t val);
    uint8_t MCPread(uint8_t reg);
    void clearInterrupt();

    uint8_t state;
    uint8_t interruptPin;
    volatile bool interrupted;
    volatile long interruptTime;
    std::function<void(uint8_t newState, uint8_t oldState)> onChange;
};


#endif
