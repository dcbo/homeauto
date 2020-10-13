/*!
 * @file mcp23017_DC.cpp
 *
 * @mainpage Darios MCP23017 Library
 *
 * @section intro_sec 
 *
 * This is a library for the MCP23017 i2c port expander
 *
 * @section author Author
 *
 * Written by Dario Carlussio, based on the Library from Limor Fried/Ladyada from Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text above must be included in any redistribution
 */

#ifdef __AVR
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif
#include "mcp23017_DC.h"

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// minihelper to keep Arduino backward compatibility
static inline void wiresend(uint8_t x, TwoWire *theWire) {
#if ARDUINO >= 100
  theWire->write((uint8_t)x);
#else
  theWire->send(x);
#endif
}

static inline uint8_t wirerecv(TwoWire *theWire) {
#if ARDUINO >= 100
  return theWire->read();
#else
  return theWire->receive();
#endif
}

/*!
 * Initializes the MCP23017 given its HW selected address, see datasheet for
 * Address selection.
 * @param addr configurable part of the address (0x20)
 * @param theWire the I2C object to use, defaults to &Wire
 * @return Returns the 0 on success, else errorcode
 */
uint8_t mcp23017::begin(uint8_t addr, TwoWire *theWire) {
  uint8_t error;
  if (addr > 7) {
    addr = 7;
  }
  i2caddr = addr;
  _wire = theWire;
  _wire->begin();
  // test if device is present
  _wire->beginTransmission(MCP23017_ADDRESS | i2caddr);
  error = _wire->endTransmission();
  if (error==0){
    // set port A and B to input
    writeRegister(MCP23017_IODIRA, 0xff);
    writeRegister(MCP23017_IODIRB, 0xff);
  }
  return (error);
}

/**
 * Initializes the default MCP23017, 
 * with 0 for the configurable part of the address (0x20)
 * @param theWire the I2C object to use, defaults to &Wire
 */
uint8_t mcp23017::begin(TwoWire *theWire) { 
    uint8_t r;
    r = begin(0, theWire); 
    return (r);
  }


/**
 * Read a single port, A or B, and return its current 8 bit value.
 * @param portb Decided what gpio to use. Should be 0 for GPIOA, and 1 for GPIOB.
 * @return Returns the b bit value of the port
 */
uint8_t mcp23017::readGPIO(uint8_t portb) {
// send cmd: read GPIOA
  _wire->beginTransmission(MCP23017_ADDRESS | i2caddr);
  if (portb == 0)
    wiresend(MCP23017_GPIOA, _wire);
  else {
    wiresend(MCP23017_GPIOB, _wire);
  }
  _wire->endTransmission();
  // get response: 
  _wire->requestFrom(MCP23017_ADDRESS | i2caddr, 1);
  return wirerecv(_wire);
}


/*!
 * Reads all 16 pins (port A and B) into a single 16 bits variable.
 * @return Returns the 16 bit variable representing all 16 pins
 */
uint16_t mcp23017::readGPIOAB() {
  uint16_t ba = 0;
  uint8_t a;
  // send cmd: read GPIOA
  _wire->beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(MCP23017_GPIOA, _wire);
  _wire->endTransmission();
  // get response: 
  _wire->requestFrom(MCP23017_ADDRESS | i2caddr, 2);
  a = wirerecv(_wire);  // GPIOA 
  ba = wirerecv(_wire); // GPIOB
  ba <<= 8;             
  ba |= a;  
  return ba;
}


/*!
 * Reads a given register
 */
uint8_t mcp23017::readRegister(uint8_t addr) {
  // read the current GPINTEN
  _wire->beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(addr, _wire);
  _wire->endTransmission();
  _wire->requestFrom(MCP23017_ADDRESS | i2caddr, 1);
  return wirerecv(_wire);
}


/*!
 * Writes all the pins in one go. This method is very useful if you are
 * implementing a multiplexed matrix and want to get a decent refresh rate.
 */
void mcp23017::writeGPIOAB(uint16_t ba) {
  _wire->beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(MCP23017_GPIOA, _wire);
  wiresend(ba & 0xFF, _wire); // GPIOA
  wiresend(ba >> 8, _wire);   // GPIOB
  _wire->endTransmission();
}


/*!
 * Writes a given register
 */
void mcp23017::writeRegister(uint8_t regAddr, uint8_t regValue) {
  // Write the register
  _wire->beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(regAddr, _wire);
  wiresend(regValue, _wire);
  _wire->endTransmission();
}


/**
 * Configures the interrupt system. both port A and B are assigned the same
 * configuration.
 * @param mirroring Mirroring will OR both INTA and INTB pins.
 * @param openDrain Opendrain will set the INT pin to value or open drain.
 * @param polarity polarity will set LOW or HIGH on interrupt.
 * Default values after Power On Reset are: (false, false, LOW)
 * If you are connecting the INTA/B pin to arduino 2/3, you should configure the
 * interupt handling as FALLING with the default configuration.
 */
void mcp23017::setupInterrupts(uint8_t mirroring, uint8_t openDrain,
                                        uint8_t polarity) {
  uint8_t ioconfValue;
  // configure the port A
  ioconfValue = readRegister(MCP23017_IOCONA);
  bitWrite(ioconfValue, 6, mirroring);
  bitWrite(ioconfValue, 2, openDrain);
  bitWrite(ioconfValue, 1, polarity);
  writeRegister(MCP23017_IOCONA, ioconfValue);
  // Configure the port B
  ioconfValue = readRegister(MCP23017_IOCONB);
  bitWrite(ioconfValue, 6, mirroring);
  bitWrite(ioconfValue, 2, openDrain);
  bitWrite(ioconfValue, 1, polarity);
  writeRegister(MCP23017_IOCONB, ioconfValue);
}

