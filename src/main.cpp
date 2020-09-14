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


/************************************************************
 * Program Configuration Control
 ************************************************************/ 
#define DO_HEARTBEAT  1
  #define HEARTBEAT 5000             // Print State Interval
#define DO_SPEED      0
  #define SPEEDRUNS     10000
  #define SPEEDUSDIVISOR (SPEEDRUNS / 1000)
  #define SPEEDBEAT     1000
#define DO_IRQ        1
  #define RESETINTERVAL 100


/************************************************************
 * Global Vars
 ************************************************************/ 
volatile bool irqFlag = false;
uint32_t lastPrint;
uint32_t lastReset;
uint8_t  laststate;


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
  Serial.print(F("- MCP23017 #"));
  Serial.print(adr);
  Serial.println(F("- [INPUT]"));
  delay(100);
  mcp.begin(adr, &Wire);  
  // Direction: INPUT
  Serial.println(F("  - Direction: INPUT"));
  delay(100);
  mcp.writeRegister(MCP23017_IODIRA, 0xff); 
  mcp.writeRegister(MCP23017_IODIRB, 0xff);   
  // Pull-UP: enable 
  Serial.println(F("  - Pull-UP: enable"));
  delay(100);
  mcp.writeRegister(MCP23017_GPPUA, 0xff);  
  mcp.writeRegister(MCP23017_GPPUB, 0xff);   
  // Input Polarity: low active = 1
  Serial.println(F("  - Input Polarity: low active"));
  delay(100);
  mcp.writeRegister(MCP23017_IPOLA, 0xff);  
  mcp.writeRegister(MCP23017_IPOLB, 0xff);   
  #if DO_IRQ
    // no Mirror, Open-Drain, LOW-active
    Serial.println(F("    - IRQ: no Mirror, Open-Drain, LOW-active"));
    delay(100);
    mcp.setupInterrupts(0, 1, 0);          
    // Interrupt Mode: on-default / on-change
    Serial.println(F("    - IRQ Mode: Change"));
    delay(100);
    mcp.writeRegister(MCP23017_INTCONA, 0x00);  // 0x00: Change
    mcp.writeRegister(MCP23017_INTCONB, 0x00);  // 0xff: Default      
    // Default Value 
    Serial.println(F("    - IRQ: Set Default Values"));
    delay(100);
    mcp.writeRegister(MCP23017_DEFVALA, 0x00);  // A
    mcp.writeRegister(MCP23017_DEFVALB, 0x00);  // B    
    // Interrupt Enable
    Serial.println(F("    - Enable Interrupts"));    
    delay(100);
    mcp.writeRegister(MCP23017_GPINTENA, 0xff); // 1: Enable 
    mcp.writeRegister(MCP23017_GPINTENB, 0x00); // 0: Disable    
    // clearInterrupts
    Serial.println(F("    - clearInterrupts"));
    delay(100);
    uint8_t i;
    i = mcp.readRegister(MCP23017_INTCAPA);
    i += mcp.readRegister(MCP23017_INTCAPB);        
  #endif // DO_IRQ 
}


/************************************************************
 * Setup 
 ************************************************************/
void setup() {        
  // Serial Port
  Serial.begin(115200);  
  Serial.println(F(""));
  Serial.println(F("####################################"));
  Serial.println(F("### Darios Homeautomation v2.0.0 ###"));
  Serial.println(F("####################################"));
  Serial.println(F("Init ..."));
  delay(1000);

  // MCP23017
  Serial.println(F("- MCP23017 #1"));
  setupInputMcp(mcp[0], 0);

  // Arduino IRQ
  #if DO_IRQ    
    Serial.print(F("- Arduino IRQ ..."));
    pinMode(INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INT_PIN), iqrHandler, FALLING);
    Serial.println(F(" done."));
  #endif // DO_IRQ
  delay(500);

  // Set I2C Speed
  Serial.print(F("- I2C: Set Speed to "));
  Serial.print(I2CSPEED);
  Serial.print(F(" ..."));
  delay(100);
  Wire.setClock(I2CSPEED);
  Serial.println(F(" done."));
  delay(500);
 
  // Global vars
  Serial.print(F("- Global Vars ... "));
  delay(100);
  laststate = 0xff;
  lastReset = millis();
  lastPrint = millis();
  irqFlag = false;
  Serial.println(F("done."));
  delay(500);

  // init finished
  Serial.println(F("Init complete, starting Main-Loop"));
  Serial.println(F("#################################"));
  delay(1000);
}


/************************************************************
 *  Print State 
 ************************************************************
 * - prints: ": -1----11 -1----11 [0x67]"      
 ************************************************************/
void printStateAB(uint16_t s) {
  uint8_t i;
  // Print Label
  Serial.print(F(": "));
  // Print Bits  
  for (i=0; i<16; i++) {
    if ((s & (1<<i)) == 0) {
      Serial.print(F("-"));
    } else {
      Serial.print(F("1"));
    }
    if (i==7) {
      Serial.print(F(" "));
    }
  }  
  Serial.print(F(" [0x"));
  // Print Value 
  Serial.print(s,HEX);
  Serial.println(F("]"));
}

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
    if (millis() - lastPrint > HEARTBEAT) {    
      lastPrint = millis();
      Serial.print(F("H"));
      printStateAB(mcp[0].readGPIOAB());
    } 
  } 
#else 
  void ReadInputs() {}  
#endif // DO_HEARTBEAT

#if DO_IRQ
  /************************************************************
   * Process IRQ 
   ************************************************************/
  void ProcessIrq() {    
    uint16_t i_port;    
    uint8_t intstate;

    // Print Value
    if (irqFlag) {
      // Print Portstate      
      Serial.print(F("I"));
      printStateAB(mcp[0].readGPIOAB());
      irqFlag = false;    
    } 

    // Arduino IRQ-Pin changed        
    intstate = digitalRead(INT_PIN);
    if (laststate != intstate) {
      laststate = intstate;
      Serial.print(F("I: "));
      Serial.println(laststate);      
    } 

    // Reset IRQ State if INT=0 (cative)
    if (!intstate){      
      if (millis() - lastReset > RESETINTERVAL) {
        lastReset = millis();
        i_port = mcp[0].readGPIOAB();  // Read Port A + B    
        if (i_port == 0) {
          // clearInterrupts    
          intstate = mcp[0].readRegister(MCP23017_INTCAPA);
          intstate = mcp[0].readRegister(MCP23017_INTCAPB);      
          Serial.println(F("I: Reset IRQ"));
        } 
      }
    }
  } 
#else 
  void ProcessIrq() {}  
#endif // DO_IRQ

/************************************************************
 * Main Loop
 ************************************************************/
void loop(){ 
  SpeedTest();
  ReadInputs();
  ProcessIrq();
} 