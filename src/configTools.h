/************************************************************
 * This File implements everything related to Configuration 
 ************************************************************
 * Factory Default Configuration is stored in Flash
 * Actual Configuration is stored in EEPROM
 ************************************************************
 * Functions implemented to
 * - GetConfiguration Params, such as:
 *   - getClickCommandFromEEprom: Read what shall be done whenn an Switch was clickef
 *   - getRollerFromEEprom: Read actual Roller Configuration
 *   - getSpecialEventFromEEprom: Read Special Events
 * - resetToFactoryDefaults: Reset Configuration to factrory default
 * - printConfig: Print Configuration stored in EEPROM 
 ************************************************************
 * ToDo-List
 * - Functions to change Configuration
 ************************************************************/ 
#ifndef _CONFIGTOOLS_H_
#define _CONFIGTOOLS_H_

#include <Arduino.h>
#include <EEPROM.h>
#include <debugOptions.h>
#include <mySettings.h>

class config {
    public:
    // public functions
    uint8_t getClickCommandFromEEprom (uint8_t clickType, uint8_t inPinNumber, uint8_t &cmd, uint8_t &par);
    void getRollerFromEEprom (uint8_t roller, uint8_t& upPin, uint8_t& downPin, uint8_t& upTime, uint8_t& downTime, uint8_t&  defaultTime);
    uint8_t getSpecialEventFromEEprom (uint8_t specialEvent, uint8_t counter);
    void resetToFactoryDefaults (void);
    void printConfig (void);

    private:
    uint8_t readFactoryDefaultTable (uint8_t FDTableNum, uint8_t FDTableValType, uint8_t FDTableEntryNum);
    uint8_t readByteFromE2PROM (uint16_t E2Adr);
    void writeByteToE2PROM (uint16_t E2Adr, uint8_t E2Val);
    void printSpecialEventsConfiguration(void);
    void printClickCommand (uint8_t cType, uint8_t inPin);
    void printClickCommandTable (uint8_t cType);
    void printRollerConfiguration(void);
};

#endif  // _CONFIGTOOLS_H_