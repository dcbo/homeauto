# define STATEUP    0B10101010
# define STATEDOWN  0B01010101
# define STATEDOWN2 0B01000000
# define STATESTOP  0B00000000
# define LED_ON     digitalWrite(13, HIGH)
# define LED_OFF    digitalWrite(13, LOW)

/************************************************************
 * Darios Homeautomatisation v2
 ************************************************************
 * Hardware:
 * - Arduino Nano
 * - 4 x I2C I/O-Expander MCP 23017
 *   - I2C-addr: 0x20 Input (16 Bit)
 *   - I2C-addr: 0x21 Input (16 Bit)
 *   - I2C-addr: 0x22 Output (16 Bit)
 *   - I2C-addr: 0x23 Output (16 Bit)
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


/************************************************************
 * Debugging Config
 ************************************************************/ 
#define DEBUG                 1  // Debug main 
#define DEBUG_ERROR           1  // Error Messages
#define DEBUG_EE_INIT         1  // Debug EEPROM Init [1142 Byte]
#define DEBUG_EE_READ         0  // Debug EEPROM Read Access
#define DEBUG_EE_WRITE        0  // Debug EEPROM Write Access
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
#define HEARTBEAT     5000             // Print State Interval
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
#define DBG_EE_WRITE      if(DEBUG_EE_WRITE)Serial 
#define DBG_EE_READ       if(DEBUG_EE_READ)Serial 


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
uint32_t g_lastResetTime;         // Used by processIrq


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
 * @param adr Address (0-7) of MCP 
 *            ATTENTION library does not use I2C-Address
 *            I2C-Address = 0x20 + adr
 *            e.g: adr=3 -> I2C-Address 0x23
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
 *            ATTENTION library does not use I2C-Address
 *            I2C-Address = 0x20 + adr
 *            e.g: adr=3 -> I2C-Address 0x23
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
  mcp.readRegister(MCP23017_INTCAPA);
  mcp.readRegister(MCP23017_INTCAPB);        
}


/************************************************************
 * Setup Output-MCP
 * @param mcp Object to be generated
 * @param adr Adress (0-7) of MCP
 *            ATTENTION library does not use I2C-Address
 *            I2C-Address = 0x20 + adr
 *            e.g: adr=3 -> I2C-Address 0x23
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
  
  // Debug LED
  pinMode(13, OUTPUT);

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
uint8_t  readFactoryDefaultTable (uint8_t FDTableNum, uint8_t FDTableValType, uint8_t FDTableEntryNum) {  
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
 * readByteFromE2PROM
 * Read one byte from EEPROM
 * @param[in] E2Adr Address to be read from
 * @returns value at EEPROM Address, 0 if address invalif 
 ************************************************************/ 
uint8_t readByteFromE2PROM (uint16_t E2Adr) {
  uint8_t E2Val;
  // read from EEPROM  
  if (E2Adr < EEPROM.length()){
    E2Val = EEPROM.read (E2Adr);  
    // Debug Output
    #if DEBUG_EE_READ
      DBG_EE_READ.print(F("EE-Read: Adr 0x"));
      if (E2Adr < 0x100) DBG_EE_READ.print(F("0"));
      if (E2Adr < 0x10) DBG_EE_READ.print(F("0"));
      DBG_EE_READ.print(E2Adr,HEX);             
      DBG_EE_READ.print(F(", Value:0x"));
      if (E2Val < 0x10) DBG_EE_READ.print(F("0"));
      DBG_EE_READ.println(E2Val,HEX);    
    #endif // DEBUG_EE_INIT
    return (E2Val);
  } else {
    DBG_ERROR.print(F("ERROR: readByteFromE2PROM failed: Address out of scope: "));
    DBG_ERROR.println(E2Adr);
    return (0);
  }   
}


void writeToE2PROM (uint16_t E2Adr, uint8_t E2Val) {
  // Debug Output
  #if DEBUG_EE_WRITE
    DBG_EE_WRITE.print(F("EE-Write: Adr 0x"));    
    if (E2Adr < 0x100) DBG_EE_WRITE.print(F("0"));
    if (E2Adr < 0x10) DBG_EE_WRITE.print(F("0"));
    DBG_EE_WRITE.print(E2Adr,HEX);             
    DBG_EE_WRITE.print(F(", Value:0x"));
    if (E2Val < 0x10) DBG_EE_WRITE.print(F("0"));
    DBG_EE_WRITE.println(E2Val,HEX);    
  #endif // DEBUG_EE_INIT
  // Write to EEPROM
  if (E2Adr < EEPROM.length()){
    EEPROM.write(E2Adr, E2Val);                
  } else {
    DBG_ERROR.print(F("ERROR: writeToE2PROM failes: Address out of scope: "));
    DBG_ERROR.println(E2Adr);
  }
}


/*
void writeToE2PROM (uint16_t E2Adr, uint16_t E2Val) {
  // Debug Output
  #if DEBUG_EE_WRITE
    DBG_EE_WRITE.print(F(" Adr:0x"));
    if (E2Adr < 0x100) DBG_EE_WRITE.print(F("0"));
    if (E2Adr < 0x10) DBG_EE_WRITE.print(F("0"));
    DBG_EE_WRITE.print(E2Adr,HEX);             
    DBG_EE_WRITE.print(F(", Value:0x"));
    if (E2Val < 0x1000) DBG_EE_WRITE.print(F("0"));
    if (E2Val < 0x100) DBG_EE_WRITE.print(F("0"));
    if (E2Val < 0x10) DBG_EE_WRITE.print(F("0"));
    DBG_EE_WRITE.println(E2Val,HEX);    
  #endif // DEBUG_EE_INIT
  // Write to EEPROM  
  if (E2Adr < (EEPROM.length()-1){
    EEPROM.write(E2Adr, E2Val);                
  } else {
    DBG_ERROR.print(F("ERROR: writeToE2PROM[16 Bit] failed: Address out of scope: "));
    DBG_ERROR.println(E2Adr);
  }
}
*/


/************************************************************
 * Copy Factory Defaults from Flash to EEPROM
 ************************************************************
 * - Convert 3 Tables and store Data to E2PROM
 *   - FactoryDefaultClickTable[][3]
 *   - FactoryDefaultDoubleClickTable[][3]
 *   - FactoryDefaultLongClickTable[][3]
 * - Store Roller Config to EEPROM
 * - Store Special Events to EEPROM
 **********************************************
 * EEPROM Layout:
 **********************************************
 * - EEPROM Size is 1024 Byte
 * - Click, Double-Click, Long-Click Events are 
 *   stored in three seperate tables.
 *   - We have 32 Inputs, so each table contains 32 actions
 *     First Table-Entry corresponds to First Input
 *   - Size of each Table Entry is 1 BYTE
 *     - Upper three Bit ((value & 0xE0)>>6) 
 *       containing the Event Type (0 to 7) 
 *       EVENT_SPECIAL, EVENT_TOGGLE, EVENT_ON, EVENT_OFF,
 *       EVENT_ROLLER_ACTION, EVENT_ROLLER_UP, EVENT_ROLLER_DOWN, EVENT_ROLLER_STOP
 *     - Lower six Bit (value & 0x3F) 
 *       containing the Parameter for the Event. In case of 
 *       - EVENT_SPECIAL 
 *         -> # of Special Event 
 *       - EVENT_TOGGLE, EVENT_ON and EVENT_OFF 
 *         -> Output Pin
 *       - EVENT_ROLLER_ACTION, EVENT_ROLLER_UP, EVENT_ROLLER_DOWN and EVENT_ROLLER_STOP
 *         -> Affected Roller Number as Bitmask (0b0011 = Roller 1+2) (0b1100 = Roller 3+4)
 **********************************************
 * - Roller Table
 *   four values for each roller are stored
 *   - PinUp: Output Pin motor UP (motor DOWN must be PinUp+1)
 *   - upTime: maximum time to run the roller comlete up [*500ms]
 *   - downTime: maximum time to run the roller comlete down [*500ms]
 *   - closeTime: time to run the roller down to night position [*500ms]
 **********************************************
 * - Special Event Table
 *   Special Event ar stored as follows
 *   - [0x70]         : Number of Special Events
 *   - [0x71]         : Number of Bytes for Special Event 1 = N1
 *   - [0x72, ...]    : all Bytes for Special Event 1 
 *   - [0x71 + N1 + 1]: Number of Bytes for Special Event 2 
 *   - [0x71 + N1 + 2]: all Bytes for Special Event 2 
 ********************************************************
 * - For all three Tables 96 Byte of EEPROM is needed.
 *   - The following EEPROM Adresses are used:
 *     - 0x00: Click on Input Pin 1
 *     - 0x20: Double Click on Input Pin 1
 *     - 0x40: Long Click on Input Pin 1
 *     - 0x5f: Long Click on Input Pin 32
 *     - 0x60: Roller Table
 *     - 0x70: Special Events Table
 ********************************************************
 * See mySettings.h forfurther Documentation 
 ************************************************************/ 
void resetToFactoryDefaults (void) {
  boolean dontStore;
  uint16_t E2Adr;
  uint8_t E2Val;  
  uint8_t entryNum;
  uint16_t SCPointerAdr;  
  uint8_t inPin;  
  uint8_t eventType;  
  uint8_t outPin;    
  uint8_t FDTableSize;
  uint8_t FDTableNum;       
  uint8_t SENum;          // # of Special Events
  uint8_t SECount;        // Counter of Special Events  
  uint8_t SCBytes;        // Bytes of Special Command
  uint8_t SCCount;        // Counter of Special Command
  uint8_t upTime;    
  uint8_t downTime;  
  uint8_t rollerPin;
  uint8_t closeTime;
  uint8_t setupValue;    
  
  // Clear E2PROM  
  // Click, Double-Click, Long-Click Tables 
  DBG_EE_INIT.print(F(" -> E2PROM - Erase Click, Double-Click and Long-ClickTables ... "));
  for (E2Adr = EE_OFFSET_CLICK; E2Adr < EE_OFFSET_ROLLER; E2Adr++) {    
    writeToE2PROM(E2Adr, 0x00);
  }    
  DBG_EE_INIT.println(F("done."));
  // Roller Table 
  DBG_EE_INIT.print(F(" -> E2PROM - Erase Roller-Tables ... "));
  for (E2Adr = EE_OFFSET_ROLLER; E2Adr < EE_OFFSET_SPECIAL_EVENT; E2Adr++) {        
    writeToE2PROM(E2Adr, 0xff);
  }    
  DBG_EE_INIT.println(F("done."));
  // Special Events (set first Elemet to 0)
  DBG_EE_INIT.print(F(" -> E2PROM - Erase Special Event Table ... "));
  writeToE2PROM(EE_OFFSET_SPECIAL_EVENT, 0x0);    
  DBG_EE_INIT.println(F("done."));
  
  // Click-Configuration one Table each loop 
  // - Loop 0: ClickTable 
  // - Loop 1: DoubleClickTable
  // - Loop 2: LongClickTable   
  for (FDTableNum = TABLE_INDEX_CLICK; FDTableNum < TABLE_INDEX_CLICK_LONG+1; FDTableNum++){    
    // get Tablesize (FDTableValType = 0 returns Tablesize)
    FDTableSize = readFactoryDefaultTable (FDTableNum, 0, 0) / 2;     
    #if DEBUG_EE_INIT
      if (FDTableNum==0) {        
        DBG_EE_INIT.println(F(" -> E2PROM - Click-Table:"));
      } else if (FDTableNum==1) {        
        DBG_EE_INIT.println(F(" -> E2PROM - Double-Click-Table:"));
      } else {        
        DBG_EE_INIT.println(F(" -> E2PROM - Long-Click-Table:"));
      }
    #endif // DEBUG_EE_INIT
    // For each Entry
    for (entryNum = 0; entryNum < FDTableSize; entryNum++ ) {
      inPin     = readFactoryDefaultTable (FDTableNum, 1, entryNum);      
      setupValue = readFactoryDefaultTable (FDTableNum, 2, entryNum);      
      eventType =  ((setupValue & 0xE0)>>5);
      outPin = (setupValue & 0x1F) ;
      #if DEBUG_EE_INIT        
        DBG_EE_INIT.print(F("    - inPin: "));
        DBG_EE_INIT.print(inPin);      
        DBG_EE_INIT.print(F(" - setupValue: "));
        DBG_EE_INIT.print(setupValue);      
        DBG_EE_INIT.print(F(" - eventType: "));
        DBG_EE_INIT.print(eventType);      
        DBG_EE_INIT.print(F(" - outPin: "));
        DBG_EE_INIT.println(outPin);      
      #endif // DEBUG_EE_INIT
      // Store only ... 
      dontStore = false;        
      // ... if eventType is from 0 to 7
      if (eventType > 7) {
         dontStore = true;
      }
      // ... if inPin is from 0 to 31
      if ( inPin > (31) ) { 
        dontStore = true; 
      }      
      // ... if outPin is from 0 to 63 if eventType == 0 (Special Event)
      if (!dontStore){
        E2Val = (eventType << 5) | (outPin & 0x1f);
        E2Adr = inPin + (FDTableNum * MCP_IN_PINS);
        writeToE2PROM(E2Adr, E2Val);
      } else {
        DBG_EE_INIT.println(F(" - ERROR - Value not stored"));
        DBG_ERROR.print(F("ERROR: Factory Default Click Table Entry #"));
        DBG_ERROR.print(entryNum);       
        DBG_ERROR.print(F(" (eventType: "));
        DBG_ERROR.print(eventType);       
        DBG_ERROR.print(F(" - inPin: "));
        DBG_ERROR.print(inPin);       
        DBG_ERROR.print(F(" - outPin: "));
        DBG_ERROR.print(outPin);       
        DBG_ERROR.println(F(")"));
      }
    }
  }    

  // Roller-Configuration Table
  // get Tablesize
  FDTableSize = readFactoryDefaultTable (TABLE_INDEX_ROLLER, 0, 0) / 4;       
  E2Adr = EE_OFFSET_ROLLER;
  // Debug Output
  #if DEBUG_EE_INIT    
    DBG_EE_INIT.println(F(" -> E2PROM - Roller-Configuration Table:"));  
    DBG_EE_INIT.print(F("    - Number of Rollers: "));    
    DBG_EE_INIT.println(FDTableSize,HEX);
  #endif // DEBUG_EE_INIT  
  // For each Entry
  for (entryNum = 0; entryNum < FDTableSize; entryNum++ ) {
    rollerPin = readFactoryDefaultTable (FDTableNum, 1, entryNum);      
    upTime   = readFactoryDefaultTable (FDTableNum, 2, entryNum);      
    downTime = readFactoryDefaultTable (FDTableNum, 3, entryNum);
    closeTime = readFactoryDefaultTable (FDTableNum, 4, entryNum);
    // Debug Output
    #if DEBUG_EE_INIT      
      DBG_EE_INIT.print(F("    - Roller "));
      DBG_EE_INIT.println(entryNum);            
      DBG_EE_INIT.print(F("      - PinUp: "));
      DBG_EE_INIT.println(rollerPin);
      DBG_EE_INIT.print(F("      - upTime: "));
      DBG_EE_INIT.print(upTime/2);
      DBG_EE_INIT.println(F("s"));
      DBG_EE_INIT.print(F("      - downTime: "));
      DBG_EE_INIT.print(downTime/2);
      DBG_EE_INIT.println(F("s"));
      DBG_EE_INIT.print(F("      - closeTime: "));      
      DBG_EE_INIT.print(closeTime/2);      
      DBG_EE_INIT.println(F("s"));
    #endif // DEBUG_EE_INIT       
    // Write Roller outPin to EEPROM    
    E2Val = rollerPin;
    writeToE2PROM(E2Adr, E2Val);           
    E2Adr++;    
    // Write upTime to EEPROM    
    E2Val = upTime;        
    writeToE2PROM(E2Adr, E2Val);
    E2Adr++;
    // Write downTime to EEPROM    
    E2Val = downTime;    
    writeToE2PROM(E2Adr, E2Val);
    E2Adr++;
    // Write closeTime to EEPROM    
    E2Val = closeTime;    
    writeToE2PROM(E2Adr, E2Val);
    E2Adr++;
  }       
  // Special Events 
  // # of Special Events     
  SENum = pgm_read_byte( &FactoryDefaultSpecialEventsTable[0]);
  #if DEBUG_EE_INIT    
    DBG_EE_INIT.println(F("EE-Init - Special Events"));
    DBG_EE_INIT.print(F("   - Number of Special Events: "));
    DBG_EE_INIT.println(SENum);    
  #endif // DEBUG_EE_INIT
  E2Adr = EE_OFFSET_SPECIAL_EVENT;
  E2Val = SENum;
  writeToE2PROM(E2Adr, E2Val);
  // Store Special Events Table
  E2Adr = EE_OFFSET_SPECIAL_EVENT + 1 + (SENum * 2);  
  entryNum = 1;
  for (SECount = 0; SECount < SENum; SECount++ ) {    
    // Pointer to Elements
    SCPointerAdr = EE_OFFSET_SPECIAL_EVENT + 1 + (SECount * 2);
    #if DEBUG_EE_INIT    
      DBG_EE_INIT.print(F("   - Special Event "));
      DBG_EE_INIT.println(SECount + 1);
      DBG_EE_INIT.print(F("     - Starts @: "));
      DBG_EE_INIT.println(E2Adr, HEX);
    #endif // DEBUG_EE_INIT    
    // Get number of Byte for this Command
    SCBytes = pgm_read_byte( &FactoryDefaultSpecialEventsTable[entryNum]);
    entryNum++;
    #if DEBUG_EE_INIT    
      DBG_EE_INIT.print(F("     - Number of Bytes: "));    
      DBG_EE_INIT.println(SCBytes);
      DBG_EE_INIT.print(F("     - Data: "));    
    #endif // DEBUG_EE_INIT
    writeToE2PROM(SCPointerAdr, E2Adr);
    for (SCCount = 0; SCCount < SCBytes; SCCount++ ) {
      E2Val = pgm_read_byte( &FactoryDefaultSpecialEventsTable[entryNum]);
      entryNum++;
      if  (SCCount != 0) {
        DBG_EE_INIT.print(F(", "));    
      }      
      DBG_EE_INIT.print(E2Val);      
      writeToE2PROM(E2Adr, E2Val);
      E2Adr++;
    }
    DBG_EE_INIT.println(F(""));    
  }  
}

/************************************************************
 * get Action from EEPROM
 ************************************************************
 * Determine what to do on a Click-Event occured
 * @param[in] clickType BUTTON_CLICK, BUTTON_CLICK_DOUBLE or BUTTON_CLICK_LONG
 * @param[in] pinNumber Number of the input Pin where the Click occured
 * @returns EVENT [0-7] and Parameter [0-31]
 *          Encoded to one Byte (EVENT) << 5 + Parameter  
 ************************************************************/
uint8_t getActionFromEEprom (uint8_t clickType, uint8_t pinNumber) {
  uint16_t E2Adr;
  uint8_t E2Val;    
  E2Adr = pinNumber + (clickType * MCP_IN_PINS);
  E2Val = 0;
  // Check Ranges
  if ((pinNumber < MCP_IN_PINS) && (clickType < BUTTON_CLICK_LONG + 1)) {
    E2Val = readByteFromE2PROM (E2Adr);    
  } 
  return (E2Val); 
}


/********************************************************
 * Special Events Table
 ********************************************************
 * Get next Command from Special Events Table
 * @param[in] specialEvent No of Special Event - STARTING WITH 1
 * @param[in,out] counter Command Byte Number  - STARTING WITH 0
 * @param[out] param Parameter for Command if it was a 2 Byte Command
 * @returns Command @ counter as 
 *          - 0 if there is no further command 
 *          - counter+1 if it was a 1 Byte Command
 *          - counter+2 if it was a 2 Byte Command
 ********************************************************
 * EEPROM Format: 
 * - Special Events are stored in one table
 *   Starting at EE_OFFSET_SPECIAL_EVENT = 0x70
 * - 0x070: Number of Special Events
 * - 0x071: Number of Bytes for 1st Special Event = S1N
 * - 0x072++: Commands of Special Event 1 =S1-C1 to S1C[S1N]
 * - Commands of Special Event are handled one after another.
 * - There are One-Byte and Multiple-Byte Commands:
 *   - One Byte Commands CCC-NNNNN with CCC=
 *     - 001: EVENT_ON,            NNNNN = Output which has to be switched on 0-31
 *     - 010: EVENT_OFF,           NNNNN = Output which has to be switched off 0-31
 *     - 011: EVENT_TOGGLE,        NNNNN = Output which has to be toggled 0-31
 *     - 100: EVENT_ROLLER_ACTION, NNNNN = Roller MASK
 *     - 101: EVENT_ROLLER_UP,     NNNNN = Roller MASK
 *     - 110: EVENT_ROLLER_DOWN,   NNNNN = Roller MASK
 *     - 111: EVENT_ROLLER_STOP,   NNNNN = Roller MASK
 *   - Multi Byte Commands 000MMMMM = 0x00-0x1f
 *     - 0x00 0xNN: CMD_SPEED              Wait 0.N Seconds after each command (max 25.5s) - 2 Byte Command
 *     - 0x01 0xNN: CMD_WAIT               Wait 0.N Seconds (max 25.5s)   - 2 Byte Command
 *     - 0x02 0xLLLLLLLL: CMD_ON_MASK      Switch ON all MASK Bits set    - 5 Byte Command
 *     - 0x03 0xLLLLLLLL: CMD_OFF_MASK     Switch OFF all MASK Bits set   - 5 Byte Command 
 ********************************************************/
uint8_t getSpecialEventFromEEprom (uint8_t specialEvent, uint8_t& counter, uint8_t& param) {
  uint16_t E2Adr;    // EEPROM Address  
  uint8_t SENum;     // Number of Special Events
  uint8_t SELength;  // Length of Special Events
  uint8_t SECmd;     // Special Events Command
  uint8_t i;         
  
  E2Adr = EE_OFFSET_SPECIAL_EVENT;  
  SENum = readByteFromE2PROM (E2Adr);
  E2Adr++;
  if (specialEvent < (SENum + 1) ){
    // find Special Event Startpoint
    for (i=1; i<specialEvent; i++){      
      SELength = readByteFromE2PROM (E2Adr);
      E2Adr += (SELength + 1);
    }      
  }
  // E2Adr points to the length of the Special Event searched for
  SELength = readByteFromE2PROM (E2Adr);  
  // Now read the [counter]-th byte
  if (counter < SELength) {
    SECmd = readByteFromE2PROM (E2Adr + counter);  
    counter++;
  }
  // Get 2nd Byte if SECmd is a two Byte Command
  if ((SECmd & 0b11100000) == 0) {
    SECmd = readByteFromE2PROM (E2Adr + counter);  
    counter++;
  }
  return (SECmd); 
}


/************************************************************
 *  Set Output Ports
 ************************************************************
 * @param[in] newOutState State to be set on Output Ports 0 to 32
 ************************************************************/
void setOutputs(uint32_t newOutState) {
  // Output only if state has changed
  if (g_lastOutState != newOutState) {
    g_lastOutState = newOutState;
    mcp[2].writeGPIOAB((uint16_t)(newOutState & 0xffff));
    mcp[3].writeGPIOAB((uint16_t)((newOutState >> 16) & 0xffff));    
  }
}


#if DEBUG_STATE
  /************************************************************
   * printStateAB
   ************************************************************
  * print 16 Bit-State as binary String 
  * e.g.: ": -1----11 -1----11 [0x67]"      
  * @param[in] s State to be printed 
  ************************************************************/
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

  /************************************************************
   * printStateABCD
   ************************************************************
  * print 32 Bit-State as binary String 
  * e.g.: ": -1----11 -1----11 -1----11 -1----11 [0x6767]"
  * @param[in] v State to be printed 
  ************************************************************/
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
   * speedTest 
   ************************************************************/
  void speedTest() {
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
  void speedTest() {}  
#endif  // DO_SPEED


#if DO_HEARTBEAT
  /************************************************************
   * Read Inputs
   ************************************************************/  
  void readInputs() {  
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
  void readInputs() {}  
#endif // DO_HEARTBEAT

/************************************************************
 * Process IRQ 
 ************************************************************/
void processIrq(void) {    
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
 * rollerUp
 ************************************************************
 * Move Roller Up (BLOCKING)
 ************************************************************/
void rollerUp(){  
  DBG.println(F("Moving Up"));
  // Start Moving Down
  LED_ON;
  mcp[2].writeGPIOAB(STATEUP);
  delay (25000);
  mcp[2].writeGPIOAB(STATESTOP);
  LED_OFF;
}

/************************************************************
 * rollerDown
 ************************************************************
 * Move Roller Down (BLOCKING)
 ************************************************************/
void rollerDown(){  
  DBG.println(F("Moving DOWN"));
  // Start Moving Down
  LED_ON;
  mcp[2].writeGPIOAB(STATEDOWN);
  delay (14000);
  mcp[2].writeGPIOAB(STATEDOWN2);
  delay (4000);  
  mcp[2].writeGPIOAB(STATESTOP);
  LED_OFF;
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
void scanButtons(void) {           
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
  
  // Do a scan
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
  } // dothisscan
}


/************************************************************
 * printClickCommand
 ************************************************************ * 
 * Prints Command stored in EEPROM for [""|Double-|Long-]Click-Event
 * @param[in] cType Click Event Type (0 to 2)
 * @param[in] inPin Input Pin
 ************************************************************/
void printClickCommand (uint8_t cType, uint8_t inPin) {
  uint8_t E2Val;
  uint8_t cmd;
  uint8_t par;
  uint8_t i;
  E2Val = getActionFromEEprom(cType, inPin);  
  cmd = (E2Val & 0b11100000) >> 5; 
  par = (E2Val & 0b00011111);
  DBG.print(F(" "));
  // inPin Number
  if (inPin < 10) {
    DBG.print(F(" "));   
  }
  DBG.print(inPin);
  DBG.print(F(": ["));    
  // stored Value in EEPROM      
  if (E2Val < 16) {
    DBG.print(F("0"));   
  }
  DBG.print(E2Val, HEX);    
  DBG.print(F("] - "));   
  if (E2Val == 0) {
    DBG.print(F("                    No Action configured"));          
  } else {
    DBG.print(F("cmd: "));   
    DBG.print(cmd);   
    DBG.print(F(" - par: "));   
    if (par<100){
      DBG.print(F(" "));   
      if (par<10){
        DBG.print(F(" "));   
      }
    } 
    DBG.print(par);   
    DBG.print(F(" - "));   

    // Print Command in clear
    switch (E2Val & 0b11100000) {    
      case EVENT_SPECIAL:
        DBG.print(F("Special Event #"));   
        break;
      case EVENT_ON:
        DBG.print(F("ON: "));     
        break;
      case EVENT_OFF:
        DBG.print(F("OFF: "));  
        break;
      case EVENT_TOGGLE:
        DBG.print(F("TOGGLE: "));  
        break;
      case EVENT_ROLLER_ACTION:
        DBG.print(F("Roller Action: "));  
        break;
      case EVENT_ROLLER_UP:
        DBG.print(F("Roller UP:     "));  
        break;
      case EVENT_ROLLER_DOWN:
        DBG.print(F("Roller Down:   "));  
        break;
      case EVENT_ROLLER_STOP:
        DBG.print(F("Roller Stop:   "));  
        break;
    }  
    // Print Param in clear
    switch (E2Val & 0b11100000) {    
      case EVENT_SPECIAL:
      case EVENT_ON:
      case EVENT_OFF:
      case EVENT_TOGGLE:
        DBG.print(par);
        break;
      case EVENT_ROLLER_ACTION:
      case EVENT_ROLLER_UP:
      case EVENT_ROLLER_DOWN:
      case EVENT_ROLLER_STOP:
        for (i=0; i<4; i++) {        
          if (par & (1<<i)) {
            DBG.print(i+1);
          } else {
            DBG.print(F("-"));
          }        
          DBG.print(F(" "));          
        }
        break;
    }
  }
}


/************************************************************
 * printClickCommandTable
 ************************************************************ * 
 * Prints Command Table of all 32 Click Events 
 * @param[in] cType Click Event type [""|Double-|Long-] 
 ************************************************************/
void printClickCommandTable (uint8_t cType) {
  uint8_t i;     
  for (i = 0; i < 32; i++) {     
    DBG.print(F("   "));
    printClickCommand (cType, i);    
    DBG.println(F(""));
  }  
}

/************************************************************
 * printConfig
 ************************************************************ * 
 * Print Config stored in EEPROM 
 * Prints the whole Configuration to Debug-Port
 ************************************************************/
void printConfig (void) {
  // Click Table  
  DBG.println(F("Click Table:"));
  printClickCommandTable(0);
  // Double-Click Table  
  DBG.println(F("Double-Click Table:"));
  printClickCommandTable(1);
  // Long-Click Table  
  DBG.println(F("Long-Click Table:"));
  printClickCommandTable(2);
  // Roller Configuration
  DBG.println(F("Roller Configuration:"));  
}


/************************************************************
 * Main Loop
 ************************************************************/
void loop(){ 
  uint32_t newout;
  //scanButtons();
  //speedTest();
  //readInputs();
  //processIrq();
  while (false){
    DBG.println(F("RollerTest"));
    rollerUp();
    delay(30000);
    //rollerDown();
    delay(30000);   
  }
  // rollerUp();
  DBG.println(F("resetToFactoryDefaults"));  
  resetToFactoryDefaults();

  DBG.println(F("printConfig"));  
  printConfig();

  DBG.println(F("delay (50000000L);"));
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