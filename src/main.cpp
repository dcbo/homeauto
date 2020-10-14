/************************************************************
 * Darios Homeautomatisation v2
 ************************************************************
 * Hardware:
 * - Arduino Nano
 * - 4 x I2C I/O-Expander MCP 23017
 *   - 0x20 Input (16 Bit)
 *   - 0x21 Input (16 Bit)
 *   - 0x22 Output (16 Bit)
 *   - 0x23 Output (16 Bit)
 ************************************************************/

/************************************************************
 * Includes
 ************************************************************/ 
#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <mcp23017_DC.h>
#include <myDefines.h>
#include <myHWconfig.h>
#include <mySettings.h>

// Debugging Config
#define DEBUG                 1  // Debug main 
#define DEBUG_ERROR           1  // Error Messages
#define DEBUG_EE_INIT         1  // Debug EEPROM Init [1142 Byte]
#define DEBUG_SETUP           1  // Debug Setup 
#define DEBUG_SETUP_MCP       0  // Debug Setup MCP   [386 Byte]
#define DEBUG_IRQ             1  // Debug IRQ
#define DEBUG_HEARTBEAT       1  // Debug Heartbeat
#define DEBUG_OUTPUT          1  // Debug Output
#define DEBUG_STATE_CHANGE    1  // Debug The Change of States
#define DEBUG_SETUP_DELAY     50 // Debug Delay during setup

/************************************************************
 * Program Configuration Control
 ************************************************************/ 
#define DO_HEARTBEAT  1
  #define HEARTBEAT 5000             // Print State Interval
#define DO_SPEED      0
  #define SPEEDRUNS     10000
  #define SPEEDUSDIVISOR (SPEEDRUNS / 1000)
  #define SPEEDBEAT     1000
  #define IRQ_RESETINTERVAL 100

/************************************************************
 * Debugging Macros use Macro "DBG...." instead of "Serial"
 ************************************************************/ 
#define DEBUG_STATE       (DEBUG_HEARTBEAT || DEBUG_IRQ || DEBUG_STATE_CHANGE)
#define DBG               if(DEBUG)Serial 
#define DBG_ERROR         if(DEBUG_ERROR)Serial 
#define DBG_SETUP         if(DEBUG_SETUP)Serial 
#define DBG_SETUP_MCP     if(DEBUG_SETUP_MCP)Serial 
#define DBG_IRQ           if(DEBUG_IRQ)Serial 
#define DBG_HEARTBEAT     if(DEBUG_HEARTBEAT)Serial 
#define DBG_OUTPUT        if(DEBUG_OUTPUT)Serial 
#define DBG_STATE         if(DEBUG_STATE)Serial 
#define DBG_STATE_CHANGE  if(DEBUG_STATE_CHANGE)Serial 
#define DBG_EE_INIT       if(DEBUG_EE_INIT)Serial 

/************************************************************
 * Global Vars
 ************************************************************/ 
volatile bool g_irqFlag = false;
boolean  g_buttonPollingActive;   //! Polling of Buttons every 10ms active
uint32_t g_lastButtonState;       //! Last State of Buttons
uint32_t g_lastButtonIrqTime;     //! Time when last IRQ was handled 
uint32_t g_lastButtonScanTime;    //! Last Time when Buttons (Inputs) habe been read
uint8_t  g_lastIntState;          //! Last State of INT0 Pin
uint32_t g_lastOutState;          //! Last State of Output Ports 
uint32_t g_lastOutTime;           //! last Time when Output Ports have ben set

uint32_t g_lastPrintTime;         // Used by Heartbeat
uint32_t g_lastResetTime;         // Used by ProcessIrq

/************************************************************
 * Objects
 ************************************************************/ 
// 4 MCP-Chips 
mcp23017 mcp[MCP_NUM];


/************************************************************
 * IRQ Handler
 ***********************************************************/
void iqrHandler() {
    g_irqFlag = true;
}

/************************************************************
 * Begin MCP
 * - initialize MCP-Object
 * - check if MCP is present
 * @param mcp Object to be generated
 * @param adr Adress (0-7) of MCP
 ************************************************************/
void beginMcp(mcp23017& mcp, uint8_t adr) {  
  uint8_t ret;
  DBG_SETUP_MCP.print(F("  - Begin: "));
  ret = mcp.begin(adr, &Wire);  
  DBG_SETUP_MCP.print(ret);
  if (ret != 0){
    DBG_SETUP_MCP.println(F(" - ERROR"));
    DBG_ERROR.print(F("ERROR: MCP23017 #"));
    DBG_ERROR.print(adr);
    DBG_ERROR.print(F(" not responding (Error: "));
    DBG_ERROR.print(ret);
    DBG_ERROR.println(F(")"));
  } else {
    DBG_SETUP_MCP.println(F(" - OK"));
  }
}
  
/************************************************************
 * Setup Input-MCP
 * - Input
 * @param mcp Object to be generated
 * @param adr Adress (0-7) of MCP
 *     uint8_t adr:   0  |   1  |   2  |   3  |   4  |   5  |   6  |   7  |
 *         I2C-adr: 0x20 | 0x21 | 0x22 | 0x23 | 0x24 | 0x25 | 0x26 | 0x27 | 
 *********************************************************** 
 * - Set Direction of all Pins to INPUT 
 * - Enable Pull-UP for all Pins
 * - Set Input Polarity to low active (GND=1)
 * - IRQ-Settings: Mirror, Open-Drain, LOW-active
 * - IRQ-Mode: on-change
 * - IRQ-Default-Value: 0x00
 * - Enable-Interrupt 
 * - clearInterrupts
 ***********************************************************/
void setupInputMcp(mcp23017& mcp, uint8_t adr) {    
  beginMcp(mcp,adr);  
  DBG_SETUP_MCP.println(F("  - Direction: INPUT"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_IODIRA, 0xff); 
  mcp.writeRegister(MCP23017_IODIRB, 0xff);     
  DBG_SETUP_MCP.println(F("  - Pull-UP: enable"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_GPPUA, 0xff);  
  mcp.writeRegister(MCP23017_GPPUB, 0xff);     
  DBG_SETUP_MCP.println(F("  - Input Polarity: low active"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_IPOLA, 0xff);  
  mcp.writeRegister(MCP23017_IPOLB, 0xff);     
  DBG_SETUP_MCP.println(F("  - IRQ: Mirror, Open-Drain, LOW-active"));
  delay(DEBUG_SETUP_DELAY);
  mcp.setupInterrupts(1, 1, 0);            
  DBG_SETUP_MCP.println(F("  - IRQ Mode: Change"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_INTCONA, 0x00);  // 0x00: Change
  mcp.writeRegister(MCP23017_INTCONB, 0x00);  // 0xff: Default        
  DBG_SETUP_MCP.println(F("  - IRQ: Set Default Values"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_DEFVALA, 0x00);  
  mcp.writeRegister(MCP23017_DEFVALB, 0x00);    
  DBG_SETUP_MCP.println(F("  - Enable Interrupts"));    
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_GPINTENA, 0xff); // 1: Enable 
  mcp.writeRegister(MCP23017_GPINTENB, 0xff); // 0: Disable    
  // clearInterrupts
  DBG_SETUP_MCP.println(F("    - clearInterrupts"));
  delay(DEBUG_SETUP_DELAY);
  // uint8_t i;  
  // i = mcp.readRegister(MCP23017_INTCAPA);
  // i += mcp.readRegister(MCP23017_INTCAPB);        
  mcp.readRegister(MCP23017_INTCAPA);
  mcp.readRegister(MCP23017_INTCAPB);        
}

/************************************************************
 * Setup Output-MCP
 * @param mcp Object to be generated
 * @param adr Adress (0-7) of MCP
 *********************************************************** 
 * - Set Direction of all Pins to Output
 * - Set all Pins to 0=GND 
 ***********************************************************/
void setupOutputMcp(mcp23017& mcp, uint8_t adr) {  
  beginMcp(mcp,adr);
  DBG_SETUP_MCP.println(F("  - Direction: OUTPUT"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_IODIRA, 0x00); 
  mcp.writeRegister(MCP23017_IODIRB, 0x00);     
  DBG_SETUP_MCP.println(F("  - Set all Pins to GND"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeGPIOAB(0x0000);  
}


/************************************************************
 * Setup 
 ************************************************************/
void setup() {        
  uint8_t i;
  uint8_t n;
  // Serial Port
  Serial.begin(115200);  
  DBG.println(F(""));
  DBG.println(F("####################################"));
  DBG.println(F("### Darios Homeautomation v2.0.0 ###"));
  DBG.println(F("####################################"));
  DBG.println(F("Init ..."));
  delay(DEBUG_SETUP_DELAY);

  // Reset all MCP23017a
  DBG_SETUP.print(F("- Resetting all MCP23017s ... "));
  pinMode(MCP_RST_PIN, OUTPUT); 
  digitalWrite(MCP_RST_PIN,LOW);
  delay(100);
  digitalWrite(MCP_RST_PIN,HIGH);
  DBG_SETUP.println(F("done."));
  
  // Setup Input MCP23017s
  n=0;
  if (MCP_IN_NUM > 0) {
    for (i=0; i<MCP_IN_NUM; i++) {
      DBG_SETUP.print(F("- MCP23017 #"));
      DBG_SETUP.print(i);
      DBG_SETUP.println(F(" - [INPUT]"));
      setupInputMcp(mcp[i], i);  
      n++;
    }
  }

  // Setup Output MCP23017s
  if (MCP_OUT_NUM > 0) {
    for (i=n; i<MCP_NUM; i++) {
      DBG_SETUP.print(F("- MCP23017 #"));
      DBG_SETUP.print(i);
      DBG_SETUP.println(F(" - [OUTPUT]"));
      setupOutputMcp(mcp[i], i);  
    }
  }
  
  // Arduino IRQ  
  DBG_SETUP.print(F("- Arduino IRQ ..."));
  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), iqrHandler, FALLING);
  DBG_SETUP.println(F(" done."));  
  delay(DEBUG_SETUP_DELAY);

  // Set I2C Speed
  DBG_SETUP.print(F("- I2C: Set Speed to "));
  DBG_SETUP.print(I2CSPEED);
  DBG_SETUP.print(F(" ..."));
  delay(DEBUG_SETUP_DELAY);
  Wire.setClock(I2CSPEED);
  DBG_SETUP.println(F(" done."));
  delay(DEBUG_SETUP_DELAY);
 
  // Global vars
  DBG_SETUP.print(F("- Global Vars ... "));
  delay(DEBUG_SETUP_DELAY);
  
  g_buttonPollingActive = false;    
  g_irqFlag = false;
  g_lastButtonIrqTime = millis();
  g_lastButtonState = 0;
  g_lastButtonScanTime = millis();
  g_lastIntState = 0xff;
  g_lastOutState = 0x00000000;
  g_lastOutTime = millis();  
  g_lastPrintTime = millis();
  g_lastResetTime = millis();    
  
  DBG_SETUP.println(F("done."));
  delay(DEBUG_SETUP_DELAY);

  // init finished
  DBG.println(F("Init complete, starting Main-Loop"));
  DBG.println(F("#################################"));
  delay(DEBUG_SETUP_DELAY);
}

/************************************************************
 * Read Factory Defaults from Flash
 * @param FDTable Table to be read [TABLE_INDEX_CLICK, TABLE_INDEX_CLICK_DOUBLE, TABLE_INDEX_CLICK_LONG, TABLE_INDEX_ROLLER]
 * @param FDTableValType Type of Value to be read [0:Tablesize else FDTable[FDTableEntryNum][FDTableValType-1]
 * @param FDTableEntryNum Entry Number to be read
 * @returns requested value 
 * To get the Tablesize from a Table read FDTableValType=0 
 ************************************************************/ 
uint8_t  ReadFactoryDefaultTable (uint8_t FDTableNum, uint8_t FDTableValType, uint8_t FDTableEntryNum) {  
  uint8_t reqVal;
  // Click Table
  if (FDTableNum == TABLE_INDEX_CLICK) {    
    if (FDTableValType != 0) {
      reqVal = pgm_read_byte( &FactoryDefaultClickTable[FDTableEntryNum][FDTableValType-1]);
    } else {
      reqVal = sizeof(FactoryDefaultClickTable);
    }
  // Double-Click Table
  } else if (FDTableNum == TABLE_INDEX_CLICK_DOUBLE) {
    if (FDTableValType != 0) {
      reqVal = pgm_read_byte( &FactoryDefaultClickDoubleTable[FDTableEntryNum][FDTableValType-1]);
    } else {
      reqVal = sizeof(FactoryDefaultClickDoubleTable);
    }
  // Long-Click Table
  } else if (FDTableNum == TABLE_INDEX_CLICK_LONG) {
    if (FDTableValType != 0) {
      reqVal = pgm_read_byte( &FactoryDefaultClickLongTable[FDTableEntryNum][FDTableValType-1]);
    } else {
      reqVal = sizeof(FactoryDefaultClickLongTable);
    }
  // Roller Table
  } else if (FDTableNum == TABLE_INDEX_ROLLER) {
    if (FDTableValType != 0) {
      reqVal = pgm_read_byte( &FactoryDefaultRollerTable[FDTableEntryNum][FDTableValType-1]);
    } else {
      reqVal = sizeof(FactoryDefaultRollerTable);
    }
  }
  return (reqVal);
}
 
/************************************************************
 * Copy Factory Defaults from Flash to EEPROM
 ************************************************************
 * Convert 3 Tables and store Data to E2PROM
 *  - FactoryDefaultClickTable[][3]
 *  - FactoryDefaultDoubleClickTable[][3]
 *  - FactoryDefaultLongClickTable[][3]
 ************************************************************ 
 * See mySettings.h forfurther Documentation *
 ************************************************************/ 
void copyFactoryDefaultsToE2PROM (void) {
  boolean dontStore;
  uint16_t E2Adr;
  uint8_t E2Val;  
  uint8_t entryNum;
  uint8_t inPin;  
  uint8_t eventType;  
  uint8_t outPin;    
  uint8_t FDTableSize;
  uint8_t FDTableNum;       
  uint8_t upTime;    
  uint8_t downTime;    
  // Clear E2PROM  
  DBG_EE_INIT.print(F(" -> E2PROM - Erase Click-Tables ..."));
  for (E2Adr = EE_OFFSET_CLICK; E2Adr < (EE_OFFSET_CLICK + (MCP_IN_NUM * 16 * 3)); E2Adr++) {    
    EEPROM.write(E2Adr, 0x00);
  }  
  DBG_EE_INIT.println(F("Completed"));
  
  // Click-Configuration one Table each loop 
  // - Loop 0: ClickTable 
  // - Loop 1: DoubleClickTable
  // - Loop 2: LongClickTable   
  for (FDTableNum = TABLE_INDEX_CLICK; FDTableNum < TABLE_INDEX_CLICK_LONG+1; FDTableNum++){    
    // get Tablesize (FDTableValType = 0 returns Tablesize)
    FDTableSize = ReadFactoryDefaultTable (FDTableNum, 0, 0) / 3;     
    #if DEBUG_EE_INIT
      if (FDTableNum==0) {
        DBG_EE_INIT.println(F("EE-Init - Click-Table:"));
      } else if (FDTableNum==1) {
        DBG_EE_INIT.println(F("EE-Init - Double-Click-Table:"));
      } else {
        DBG_EE_INIT.println(F("EE-Init - Long-Click-Table:"));
      }
    #endif // DEBUG_EE_INIT
    // For each Entry
    for (entryNum = 0; entryNum < FDTableSize; entryNum++ ) {
      inPin     = ReadFactoryDefaultTable (FDTableNum, 1, entryNum);      
      eventType = ReadFactoryDefaultTable (FDTableNum, 2, entryNum);
      outPin    = ReadFactoryDefaultTable (FDTableNum, 3, entryNum);
      #if DEBUG_EE_INIT
        DBG_EE_INIT.print(F("EE-Init   - #:"));
        DBG_EE_INIT.print(entryNum);      
        DBG_EE_INIT.print(F(" - inPin:"));
        DBG_EE_INIT.print(inPin);      
        DBG_EE_INIT.print(F(" - eventType:"));
        DBG_EE_INIT.print(eventType);      
        DBG_EE_INIT.print(F(" - outPin:"));
        DBG_EE_INIT.print(outPin);      
      #endif // DEBUG_EE_INIT
      // Store only ... 
      dontStore = false;        
      // ... if eventType is from 0 to 3
      if (eventType > 3) {
         dontStore = true;
      }
      // ... if inPin is from 0 to MaxInputPins-1
      if ( inPin > (MCP_IN_PINS-1) ) { 
        dontStore = true; 
      }
      // ... if outPin is from 0 to MaxOutputPins-1 if eventType != 0 (ON, OFF or TOGGLE)
      if ( eventType != 0 ) {                 
        if ( inPin > (MCP_OUT_PINS-1) ) { 
          dontStore = true; 
        } 
      // ... if outPin is from 0 to 63 if eventType == 0 (Special Event)    
      } else {          
        if (inPin > 63) { 
          dontStore = true; 
        } 
      }    
      if (!dontStore){
        E2Val = (eventType << 6) | (outPin & 0x3f);
        E2Adr = inPin + (FDTableNum * MCP_IN_PINS);
        #if DEBUG_EE_INIT
          // Debug Output
          DBG_EE_INIT.print(F(" -> Adr:0x"));
          if (E2Adr < 0x10) DBG_EE_INIT.print(F("0"));
          DBG_EE_INIT.print(E2Adr,HEX);             
          DBG_EE_INIT.print(F(", Value:0x"));
          if (E2Val < 0x10) DBG_EE_INIT.print(F("0"));
          DBG_EE_INIT.println(E2Val,HEX);    
        #endif // DEBUG_EE_INIT
        // Write to EEPROM
        EEPROM.write(E2Adr, E2Val);                
      } else {
        DBG_EE_INIT.println(F(" - ERROR - Value not stored"));
        DBG_ERROR.print(F("ERROR: Factory Default Click Table Entry #"));
        DBG_ERROR.print(entryNum);       
        DBG_ERROR.print(F(" (eventType:"));
        DBG_ERROR.print(eventType);       
        DBG_ERROR.print(F(" - inPin:"));
        DBG_ERROR.print(inPin);       
        DBG_ERROR.print(F(" - outPin:"));
        DBG_ERROR.print(outPin);       
        DBG_ERROR.println(F(")"));
      }
    }
  }    
  // Roller-Configuration Table
  // get Tablesize
  FDTableSize = ReadFactoryDefaultTable (TABLE_INDEX_ROLLER, 0, 0) / 2;       
  // Debug Output
  #if DEBUG_EE_INIT    
    DBG_EE_INIT.print(F("EE-Init - Number of Rollers"));
    DBG_EE_INIT.print(F(" -> Adr:0x"));           
    DBG_EE_INIT.print(EE_OFFSET_ROLL_NUM,HEX);
    DBG_EE_INIT.print(F(", Value:0x"));
    DBG_EE_INIT.println(FDTableSize,HEX);
  #endif // DEBUG_EE_INIT
  // Store Number of Rollers to EEPROM
  EEPROM.write(EE_OFFSET_ROLL_NUM, FDTableSize);     
  // Debug Output
  #if DEBUG_EE_INIT
    DBG_EE_INIT.print(F("EE-Init - Pointer to first Element of Roller Table"));    
    DBG_EE_INIT.print(F(" -> Adr:0x"));           
    DBG_EE_INIT.print(EE_OFFSET_ROLL_ADR,HEX);
    DBG_EE_INIT.print(F(", Value:0x"));
    DBG_EE_INIT.println(EE_OFFSET_BEGIN_VARSPACE,HEX);
  #endif // DEBUG_EE_INIT
  // Store Pointer to first Element of Roller Table 
  EEPROM.write(EE_OFFSET_ROLL_ADR, EE_OFFSET_BEGIN_VARSPACE);
  E2Adr = EE_OFFSET_BEGIN_VARSPACE;
  // For each Entry
  for (entryNum = 0; entryNum < FDTableSize; entryNum++ ) {
    upTime   = ReadFactoryDefaultTable (FDTableNum, 1, entryNum);      
    downTime = ReadFactoryDefaultTable (FDTableNum, 2, entryNum);
    // Debug Output
    #if DEBUG_EE_INIT      
      DBG_EE_INIT.print(F("EE-Roller   - #:"));
      DBG_EE_INIT.print(entryNum);            
      DBG_EE_INIT.print(F(" - upTime:"));
      DBG_EE_INIT.print(upTime);
      DBG_EE_INIT.print(F(" - downTime:"));
      DBG_EE_INIT.print(downTime);
    #endif // DEBUG_EE_INIT
    E2Val = upTime;
    // Debug Output
    #if DEBUG_EE_INIT      
      DBG_EE_INIT.print(F(" -> Adr:0x"));      
      DBG_EE_INIT.print(E2Adr,HEX);             
      DBG_EE_INIT.print(F(", Value:0x"));
      if (E2Val < 0x10) DBG_EE_INIT.print(F("0"));
      DBG_EE_INIT.print(E2Val,HEX);    
    #endif // DEBUG_EE_INIT
    // Write upTime to EEPROM
    EEPROM.write(E2Adr, E2Val);
    E2Adr++;
    E2Val = downTime;
    // Debug Output
    #if DEBUG_EE_INIT      
      DBG_EE_INIT.print(F(" - Adr:0x"));
      if (E2Adr < 0x10) DBG_EE_INIT.print(F("0"));
      DBG_EE_INIT.print(E2Adr,HEX);             
      DBG_EE_INIT.print(F(", Value:0x"));
      if (E2Val < 0x10) DBG_EE_INIT.print(F("0"));
      DBG_EE_INIT.println(E2Val,HEX);
    #endif // DEBUG_EE_INIT
    // Write downTime to EEPROM
    EEPROM.write(E2Adr, E2Val);
    E2Adr++;    
  }     
  // Debug Output
  #if DEBUG_EE_INIT    
    DBG_EE_INIT.print(F("EE-Init - Pointer to first Element of Special Events Table"));    
    DBG_EE_INIT.print(F(" -> Adr:0x"));           
    DBG_EE_INIT.print(EE_OFFSET_SPECIAL_EVENT_ADR,HEX);
    DBG_EE_INIT.print(F(", Value:0x"));
    if (E2Val < 0x10) DBG_EE_INIT.print(F("0"));
    DBG_EE_INIT.println(E2Adr,HEX);
  #endif // DEBUG_EE_INIT
  // Store Pointer to first Element of Special Events Table
  EEPROM.write(EE_OFFSET_SPECIAL_EVENT_ADR, E2Adr);    
}


/************************************************************
 *  Set Output Ports
 ************************************************************
 *
 ************************************************************/
void SetOutputs(uint32_t newOutState) {
  // Output only if state has changed
  if (g_lastOutState != newOutState) {
    g_lastOutState = newOutState;
    mcp[2].writeGPIOAB((uint16_t)(newOutState && 0xffff));
    mcp[3].writeGPIOAB((uint16_t)(newOutState >> 16));
  }
}


/************************************************************
 *  Print State 
 ************************************************************
 * - prints: ": -1----11 -1----11 [0x67]"      
 ************************************************************/
#if DEBUG_STATE
  void printStateAB(uint16_t s) {
    uint8_t i;
    // Print Label
    DBG_STATE.print(F(": "));
    // Print Bits  
    for (i=0; i<16; i++) {
      if ((s & (1<<i)) == 0) {
        DBG_STATE.print(F("-"));
      } else {
        DBG_STATE.print(F("1"));
      }
      if (i==7) {
        DBG_STATE.print(F(" "));
      }
    }  
    DBG_STATE.print(F(" [0x"));
    // Print Value 
    DBG_STATE.print(s,HEX);
    DBG_STATE.println(F("]"));
  }

  void printStateABCD(uint32_t v) {
    uint8_t i;        
    // Print Bits  
    for (i=0; i<32; i++) {      
      if ((v & (1L<<i)) == 0) {
        DBG_STATE.print(F("-"));        
      } else {
        DBG_STATE.print(F("1"));
      }
      if (i%8==7) {
        DBG_STATE.print(F(" "));
      }
    }  
    DBG_STATE.print(F(" [0x"));
    // Print Value 
    DBG_STATE.print(v,HEX);
    DBG_STATE.println(F("]"));
  }
#else
  void printStateAB(uint16_t s) {};
  void printStateABCD(uint32_t s) {};
#endif  // DEBUG_STATE


#if DO_SPEED
  /************************************************************
   * Speedtest 
   ************************************************************/
  void SpeedTest() {
    uint32_t i;
    uint8_t  v8;
    uint16_t v16;
    uint32_t startT;
    uint32_t endT;
    uint16_t us16bit;
    uint16_t us8bit;
    if (millis() - lastPrint > SPEEDBEAT) {          
      Serial.println(F("-------------------------------------"));
      Serial.println(F("Speedtest started ..."));
      // 8 Bit Mode
      Serial.println(F("1: 8 Bit Mode (read MCP-Registers seperate)"));
      startT = millis();
      for (i=0; i<SPEEDRUNS; i++) {
        v8 = mcp[0].readGPIO(0);                // Read Port A + B
        v8 = mcp[0].readGPIO(1);                // Read Port A + B          
      }     
      endT = millis();    
      us8bit = (endT-startT) / SPEEDUSDIVISOR;
      Serial.print(F("   Started: "));     Serial.print(startT);  
      Serial.print(F(" - Finished: "));    Serial.print(endT);  
      Serial.print(F(" - Dauer: "));       Serial.println(endT-startT);              
      Serial.println(F("-------------------------------------"));
      
      // 16 Bit Mode
      Serial.println(F("2: 16 Bit Mode (read both MCP-Registers in one call)"));
      startT = millis();
      for (i=0; i<SPEEDRUNS; i++) {      
        v16 = mcp[0].readGPIOAB();                // Read Port A + B    
      } 
      endT = millis();    
      us16bit = (endT-startT) / SPEEDUSDIVISOR;
      Serial.print(F("   Started: "));     Serial.print(startT);  
      Serial.print(F(" - Finished: "));    Serial.print(endT);  
      Serial.print(F(" - Dauer: "));       Serial.println(endT-startT);  
      Serial.println(F("-------------------------------------"));
      Serial.print(F("  I2C-Speed: "));  Serial.println(I2CSPEED);
      Serial.print(F("      8-Bit: "));  Serial.print(us8bit);   Serial.println(F("us per Sample"));
      Serial.print(F("     16-Bit: "));  Serial.print(us16bit);  Serial.println(F("us per Sample"));
      Serial.println(F("--------------------------------------------------------------------------"));
      lastPrint = millis();
      // to surpress "unused Variable" warnings
      v16 = v8++;
      v8 = (v16 | 0xff);
    }
  }
#else 
  void SpeedTest() {}  
#endif  // DO_SPEED


#if DO_HEARTBEAT
  /************************************************************
   * Read Inputs
   ************************************************************/  
  void ReadInputs() {  
    if (millis() - g_lastPrintTime > HEARTBEAT) {    
      g_lastPrintTime = millis();
      #if DEBUG_HEARTBEAT        
        DBG_HEARTBEAT.print(F("H-0"));
        printStateAB(mcp[0].readGPIOAB());      
        DBG_HEARTBEAT.print(F("H-1"));
        printStateAB(mcp[1].readGPIOAB());      
      #endif // DEBUG_HEARTBEAT
    } 
  } 
#else 
  void ReadInputs() {}  
#endif // DO_HEARTBEAT

/************************************************************
 * Process IRQ 
 ************************************************************/
void ProcessIrq(void) {    
  uint16_t i_port;    
  uint8_t intstate;

  // Print Value
  if (g_irqFlag) {
    // Print Portstate      
    #if DEBUG_IRQ   
      DBG_IRQ.print(F("I-0"));
      printStateAB(mcp[0].readGPIOAB());      
      DBG_IRQ.print(F("I-1"));
      printStateAB(mcp[1].readGPIOAB());      
    #endif //  DEBUG_IRQ   
    g_irqFlag = false;    
  } 
  delay(100);

  // Arduino IRQ-Pin changed        
  intstate = digitalRead(INT_PIN);
  if (g_lastIntState != intstate) {
    g_lastIntState = intstate;
    #if DEBUG_IRQ   
      DBG_IRQ.print(F("Int: "));
      DBG_IRQ.println(g_lastIntState);      
    #endif // DEBUG_IRQ   
  } 

  // Reset IRQ State if INT=0 (cative)
  if (!intstate){      
    if (millis() - g_lastResetTime > IRQ_RESETINTERVAL) {
      g_lastResetTime = millis();
      i_port = mcp[0].readGPIOAB();  // Read Port A + B    
      if (i_port == 0) {
        // clearInterrupts    
        intstate = mcp[0].readRegister(MCP23017_INTCAPA);
        intstate = mcp[0].readRegister(MCP23017_INTCAPB);      
        #if DEBUG_IRQ   
          DBG_IRQ.println(F("I-0: Reset IRQ"));                  
        #endif // DEBUG_IRQ             
      } 
      i_port = mcp[1].readGPIOAB();  // Read Port A + B    
      if (i_port == 0) {
        // clearInterrupts    
        intstate = mcp[1].readRegister(MCP23017_INTCAPA);
        intstate = mcp[1].readRegister(MCP23017_INTCAPB);
        #if DEBUG_IRQ
          DBG_IRQ.println(F("I-1: Reset IRQ"));
        #endif // DEBUG_IRQ
      } 
    }
  }
} 



/************************************************************
 * Scan Input Buttons
 ************************************************************
 * Read Input-State if
 *  - IRQ occured since last call                         [1]
 *  - after IRQ occured                                   [2] 
 *    - every BUTTON_SCANINT [ms]                         [3]
 *    - untill all inputs=0 for at least BUTTON_SCANTIME  [4]
 ***********************************************************/
void ScanButtons(void) {           
  uint32_t thisstate;   // state of this scan
  boolean dothisscan;   // scan this time
  
  dothisscan = false;
  thisstate = 0x0000;
    
  // IRQ occured [1]
  if (g_irqFlag) {     
    dothisscan = true;        
    g_irqFlag = false;    
    g_buttonPollingActive = true;
    g_lastButtonIrqTime = millis();
  // If scan is still active [2]
  } else if (g_buttonPollingActive) {
    // scan every BUTTON_SCANINT[ms]  [3]
    if (millis() - g_lastButtonIrqTime > BUTTON_SCANINT) {
      dothisscan = true;              
    }
  }
  
  if (dothisscan) {     
    // Read all GPIO Registers        
    thisstate = (uint32_t)mcp[0].readGPIOAB() + ((uint32_t)mcp[1].readGPIOAB() << 16);
        
    // State changed?
    if (thisstate != g_lastButtonState) {    
      g_lastButtonState = thisstate;      
      DBG_STATE_CHANGE.print(F("Scan: "));
      printStateABCD(thisstate);

      // Reset MCP IRQ Registers, if all Inputs = 0 
      if (thisstate == 0) {
        // Reset IRQ 
        DBG_IRQ.println(F("Reseting IRQs"));
        mcp[0].readRegister(MCP23017_INTCAPA);
        mcp[0].readRegister(MCP23017_INTCAPB);      
        mcp[1].readRegister(MCP23017_INTCAPA);
        mcp[1].readRegister(MCP23017_INTCAPB);
        // End Scan [4]
        if (millis() - g_lastButtonIrqTime > BUTTON_SCANINT) {
          g_buttonPollingActive = false;      
        }
      }
    }    
  }
}


/************************************************************
 * Main Loop
 ************************************************************/
void loop(){ 
  uint32_t newout;
  // ScanButtons();
  //SpeedTest();
  //ReadInputs();
  //ProcessIrq();

  copyFactoryDefaultsToE2PROM();
  delay (50000000L);

  if (millis() - g_lastOutTime > 50000000) {
    
    g_lastOutTime = millis();
    g_lastButtonState++;
    if (g_lastButtonState == 1){
      newout = 0xFF00;      
    } else {
      newout = 0x00FF;
      g_lastButtonState =0;
    }
    DBG_OUTPUT.print(F("Out: "));
    DBG_OUTPUT.println(newout,HEX);
    mcp[2].writeGPIOAB(newout);
    mcp[3].writeGPIOAB(newout);
    //MCP23017_GPIOA
    // mcp[3].writeGPIOAB(newout);
  }
} 