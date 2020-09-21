/************************************************************
 * Darios Homeautomatisation v2
 ************************************************************
 * Hardware:
 * - Arduino Nano
 * - 4 x I2C I/O-Expander MCP 23017
 *   - 2 x MCP 23017 as Intput (16 Bit)
 *     - 32 Buttons (common GND)
 *   - 2 x MCP 23017 as Output (16 Bit)
 *     -  4 Rollers (each 2 Outputs: on, dir)
 *     - 24 Devices (lights, power outlets)
 ************************************************************/

/************************************************************
 * Includes
 ************************************************************/ 
#include <Arduino.h>
#include <Wire.h>
#include <mcp23017_DC.h>


/************************************************************
 * Defines
 ************************************************************/ 
#define I2CSPEED         800000  // 800kHz -> 127us to read all 16 GPIOs from one MCP23017
#define INT_PIN               2  // Interupt Pin

// Constants
#define BUTTON_T0            20  // <T0= No Klick (Noise)  20ms (T0-1)*10ms (max   30ms)
#define BUTTON_T1          1000  // >T1= Long Klick       950ms (T1-1)*10ms (max 1000ms)
#define BUTTON_T2           200  // <T2= Double Klick     190ms (T2-1)*10ms (max  200ms)
#define BUTTON_SCANTIME     100  // Stop Buttonaction

// Scenes
#define SCENE_ALL_OFF         0  // Turn everything OFF
#define SCENE_ALL_ON          1  // Turn everything ON
#define SCENE_LEAVING         2  // Leaving the House
#define SCENE_BED             3  // Scene Bett: alles aus bis auf Balkonlicht
#define SCENE_BED_delay       4  // Scene Bett Delay: alles aus bis auf Balkonlicht, Schlafzimmer 30s an

// Timer
#define MAX_TIMER             8
#define tm_2L1_off            0
#define tm_ROL01_stop         1
#define tm_ROL23_stop         2
#define tm_L5_off             3

// Button Event Types
#define EVENT_CLICK           0
#define EVENT_CLICK_DOUBLE    1
#define EVENT_CLICK_LONG      2

// Roller Action Types
#define ROLL_STOP             0
#define ROLL_START_UP         1
#define ROLL_START_DOWN       2
#define ROLL_START_OPPOSITE   3
#define ROLL_START_SAME       4
#define ROLL_CLICK            5
#define ROLL_TICK             6

// Debugging Config
#define DEBUG                 1  // Debug main 
#define DEBUG_SETUP           1  // Debug Setup 
#define DEBUG_SETUP_MCP       1  // Debug Setup MCP
#define DEBUG_IRQ             1  // Debug IRQ
#define DEBUG_HEARTBEAT       1  // Debug Heartbeat
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
#define DBG_SETUP         if(DEBUG_SETUP)Serial 
#define DBG_SETUP_MCP     if(DEBUG_SETUP_MCP)Serial 
#define DBG_IRQ           if(DEBUG_IRQ)Serial 
#define DBG_HEARTBEAT     if(DEBUG_HEARTBEAT)Serial 
#define DBG_STATE         if(DEBUG_STATE)Serial 
#define DBG_STATE_CHANGE  if(DEBUG_STATE_CHANGE)Serial 


/************************************************************
 * Global Vars
 ************************************************************/ 
volatile bool irqFlag = false;
uint32_t g_lastPrint;
uint32_t g_lastReset;
uint8_t  g_laststate;
boolean  g_button_scan_active;
uint32_t g_button_scan_timer;
uint32_t g_button_last_irq;
uint32_t g_button_scan_state = 0;       // State of Buttons
uint32_t g_button_scan_laststate = 0;   // Last State of Buttons

/************************************************************
 * Objects
 ************************************************************/ 
// 4 MCP-Chips 
mcp23017 mcp[4];


/************************************************************
 * IRQ Handler
 ***********************************************************/
void iqrHandler() {
    irqFlag = true;
}


/************************************************************
 * Setup one Input-MCP
 ***********************************************************/
void setupInputMcp(mcp23017& mcp, uint8_t adr) {  
  mcp.begin(adr, &Wire);  
  // Direction: INPUT
  DBG_SETUP_MCP.println(F("  - Direction: INPUT"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_IODIRA, 0xff); 
  mcp.writeRegister(MCP23017_IODIRB, 0xff);   
  // Pull-UP: enable 
  DBG_SETUP_MCP.println(F("  - Pull-UP: enable"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_GPPUA, 0xff);  
  mcp.writeRegister(MCP23017_GPPUB, 0xff);   
  // Input Polarity: low active = 1
  DBG_SETUP_MCP.println(F("  - Input Polarity: low active"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_IPOLA, 0xff);  
  mcp.writeRegister(MCP23017_IPOLB, 0xff);   
  
  // Setup IRQ
  // no Mirror, Open-Drain, LOW-active
  DBG_SETUP_MCP.println(F("    - IRQ: Mirror, Open-Drain, LOW-active"));
  delay(DEBUG_SETUP_DELAY);
  mcp.setupInterrupts(1, 1, 0);          
  // Interrupt Mode: on-default / on-change
  DBG_SETUP_MCP.println(F("    - IRQ Mode: Change"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_INTCONA, 0x00);  // 0x00: Change
  mcp.writeRegister(MCP23017_INTCONB, 0x00);  // 0xff: Default      
  // Default Value 
  DBG_SETUP_MCP.println(F("    - IRQ: Set Default Values"));
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_DEFVALA, 0x00);  // A
  mcp.writeRegister(MCP23017_DEFVALB, 0x00);  // B    
  // Interrupt Enable
  DBG_SETUP_MCP.println(F("    - Enable Interrupts"));    
  delay(DEBUG_SETUP_DELAY);
  mcp.writeRegister(MCP23017_GPINTENA, 0xff); // 1: Enable 
  mcp.writeRegister(MCP23017_GPINTENB, 0xff); // 0: Disable    
  // clearInterrupts
  DBG_SETUP_MCP.println(F("    - clearInterrupts"));
  delay(DEBUG_SETUP_DELAY);
  uint8_t i;
  i = mcp.readRegister(MCP23017_INTCAPA);
  i += mcp.readRegister(MCP23017_INTCAPB);        
}



/************************************************************
 * Setup 
 ************************************************************/
void setup() {        
  // Serial Port
  Serial.begin(115200);  
  DBG.println(F(""));
  DBG.println(F("####################################"));
  DBG.println(F("### Darios Homeautomation v2.0.0 ###"));
  DBG.println(F("####################################"));
  DBG.println(F("Init ..."));
  delay(DEBUG_SETUP_DELAY);

  // MCP23017
  DBG_SETUP.println(F("- MCP23017 #0 - [INPUT]"));
  setupInputMcp(mcp[0], 0);
  DBG_SETUP.println(F("- MCP23017 #1 - [INPUT]"));
  setupInputMcp(mcp[1], 1);

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
  g_laststate = 0xff;
  g_button_scan_active = false;
  g_button_scan_timer = millis();
  g_button_scan_state = 0;
  g_button_scan_laststate = 0;
  g_lastReset = millis();
  g_lastPrint = millis();
  g_button_last_irq = millis();
  irqFlag = false;
  DBG_SETUP.println(F("done."));
  delay(DEBUG_SETUP_DELAY);

  // init finished
  DBG.println(F("Init complete, starting Main-Loop"));
  DBG.println(F("#################################"));
  delay(DEBUG_SETUP_DELAY);
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
    if (millis() - g_lastPrint > HEARTBEAT) {    
      g_lastPrint = millis();
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
  if (irqFlag) {
    // Print Portstate      
    #if DEBUG_IRQ   
      DBG_IRQ.print(F("I-0"));
      printStateAB(mcp[0].readGPIOAB());      
      DBG_IRQ.print(F("I-1"));
      printStateAB(mcp[1].readGPIOAB());      
    #endif //  DEBUG_IRQ   
    irqFlag = false;    
  } 
  delay(100);

  // Arduino IRQ-Pin changed        
  intstate = digitalRead(INT_PIN);
  if (g_laststate != intstate) {
    g_laststate = intstate;
    #if DEBUG_IRQ   
      DBG_IRQ.print(F("Int: "));
      DBG_IRQ.println(g_laststate);      
    #endif // DEBUG_IRQ   
  } 

  // Reset IRQ State if INT=0 (cative)
  if (!intstate){      
    if (millis() - g_lastReset > IRQ_RESETINTERVAL) {
      g_lastReset = millis();
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
 ***********************************************************/
void ScanButtons(void) {           
  uint32_t thisstate;  
  boolean doscan;
  
  // Scan only if
  // - IRQ occured 
  // TODO - every 10ms after IRQ occured for 5s
  doscan = false;
  
  // IRQ occured 
  if (irqFlag) {     
    doscan = true;        
    irqFlag = false;    
    g_button_scan_active = true;
    g_button_last_irq = millis();
  } 
  
  // If scan is still active, scann every BUTTON_SCANTIME [ms]
  if (g_button_scan_active) {
    if (millis() - g_button_last_irq > BUTTON_SCANTIME) {
      doscan = true;        
      g_button_last_irq = millis();    
    }
  }
  
  if (doscan) {     
    // Read all GPIO Registers        
    thisstate = (uint32_t)mcp[0].readGPIOAB() + ((uint32_t)mcp[1].readGPIOAB() << 16);
        
    // State changed?
    if (thisstate != g_button_scan_laststate) {    
      g_button_scan_laststate = thisstate;
      DBG_STATE_CHANGE.print(F("Scan: "));
      printStateABCD(thisstate);
    }

    // If all Inputs = 0 AND All ButtonStates = idle    
    if (thisstate == 0) {
      // End Scan 
      g_button_scan_active = false;
      // Reset IRQ 
      DBG_IRQ.println(F("Reseting IRQs"));
      mcp[0].readRegister(MCP23017_INTCAPA);
      mcp[0].readRegister(MCP23017_INTCAPB);      
      mcp[1].readRegister(MCP23017_INTCAPA);
      mcp[1].readRegister(MCP23017_INTCAPB);
    }
  }
}


/************************************************************
 * Main Loop
 ************************************************************/
void loop(){ 
  ScanButtons();
  //SpeedTest();
  //ReadInputs();
  //ProcessIrq();
  
} 