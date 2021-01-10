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
#include <debugOptions.h>
// #include <EEPROM.h>
#include <Wire.h>
#include <mcp23017_DC.h>
#include <mySettings.h>
#include <configTools.h>

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

// Access Configuration
config myconfig;

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
 * @param[in] mcp Object to be generated
 * @param[in] adr Address (0-7) of MCP 
 *                ATTENTION library does not use I2C-Address
 *                I2C-Address = 0x20 + adr
 *                e.g: adr=3 -> I2C-Address 0x23
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
 * @param[in] mcp Object to be generated
 * @param[in] adr Adress (0-7) of MCP
 *                ATTENTION library does not use I2C-Address
 *                I2C-Address = 0x20 + adr
 *                e.g: adr=3 -> I2C-Address 0x23
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
 * @param[in] mcp Object to be generated
 * @param[in] adr Adress (0-7) of MCP
 *                ATTENTION library does not use I2C-Address
 *                I2C-Address = 0x20 + adr
 *                e.g: adr=3 -> I2C-Address 0x23
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
 *  Set Output Ports
 ************************************************************
 * The actual state is stored in g_lastOutState. 
 * The output occures only, if the new state is different.
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
#else
  void printStateAB(uint16_t s) {};
#endif  // DEBUG_STATE


#if DEBUG_STATE
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
  // Reset IRQ State if INT=0 (active)
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
  // init vars
  dothisscan = false;
  thisstate = 0x0000;    
  // IRQ occured [1]
  if (g_irqFlag) {     
    dothisscan = true;        
    g_irqFlag = false;    
    g_buttonPollingActive = true;
    g_lastButtonIrqTime = millis();    
  } else if (g_buttonPollingActive) {
    // scan is still active [2]    
    if (millis() - g_lastButtonIrqTime > BUTTON_SCANINT) {
      // scan every BUTTON_SCANINT[ms]  [3]
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
  } 
}

/************************************************************
 * Main Loop
 ************************************************************/
void loop(){ 
  
  // scanButtons();

  // speedTest();
  // readInputs();
  // processIrq();  
  

  
  rollerDown();
  // rollerUp();
  
  DBG.println(F("\n\nresetToFactoryDefaults"));    
  myconfig.resetToFactoryDefaults();

  DBG.println(F("\n\nprintConfig"));    
  myconfig.printConfig();

  
  // end here
  while (true) {
    DBG.println(F("delay (3600000L);"));
    delay (3600000L);
  }

  // uint32_t newout;
  //if (millis() - g_lastOutTime > 50000000) {    
    //g_lastOutTime = millis();
    //g_lastButtonState++;
    //if (g_lastButtonState == 1){
    //  newout = 0xFF00;      
    //} else {
    //  newout = 0x00FF;
    //  g_lastButtonState =0;
    //}
    //DBG_OUTPUT.print(F("Out: "));
    //DBG_OUTPUT.println(newout,HEX);
    //mcp[2].writeGPIOAB(newout);
    //mcp[3].writeGPIOAB(newout);
    //MCP23017_GPIOA
    // mcp[3].writeGPIOAB(newout);
 //  }
} 