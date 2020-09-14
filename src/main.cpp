#include <Arduino.h>
#include <Wire.h>
#include <mcp23017_DC.h>


/******************************************************
 *** Speed Test:
 ******************************************************
 *** Read all 16 GPIOs
 *** -  8-Bit mode: read each GPIO Register seperate
 *** - 16-Bit mode: read bith GPIO Register in one call
 *** Do 10.000 iterations 
 *** calculte time to read all 16 GPIOs in [us]
 ******************************************************
 *** Results:
 ******************************************************
  I2CSPEED      8-Bit Mode    16-Bit Mode
    100000          899          550
    200000          511          307 
    300000          368          222 
    400000          310          186 
    600000          241          143 
    800000          210          125 
   1000000          190          113 
   1200000         2711         1656  
*/
#define I2CSPEED 800000
#define MCP_ADDR  0x20          // (A2/A1/A0 = LOW) 
#define INT_PIN   2             // Interupt Pin

#define DO_HEARTBEAT  0
  #define HEARTBEAT 5000             // Print State Interval
#define DO_SPEED      0
  #define SPEEDRUNS     10000
  #define SPEEDUSDIVISOR (SPEEDRUNS / 1000)
  #define SPEEDBEAT     1000
#define DO_IRQ        1
#define RESETINTERVAL 100

volatile bool irqFlag = false;
uint32_t lastPrint;
uint32_t lastReset;
uint8_t  laststate;

mcp23017 mcp[2];
// mcp23017 mcp2;
// mcp23017 mcp3;
// mcp23017 mcp4;


#if DO_IRQ
/***********
 IRQ Handler
 ***********/
void iqrHandler() {
    irqFlag = true;
}
#endif // DO_IRQ


/***************
 Setup Input-MCP
 ***************/
void setupInputMcp(mcp23017& mcp, uint8_t adr) {
  mcp.begin(adr, &Wire);  
  // Direction: INPUT
  Serial.println("  - Direction: INPUT");
  delay(100);
  mcp.writeRegister(MCP23017_IODIRA, 0xff); 
  mcp.writeRegister(MCP23017_IODIRB, 0xff);   
  // Pull-UP: enable 
  Serial.println("  - Pull-UP: enable");
  delay(100);
  mcp.writeRegister(MCP23017_GPPUA, 0xff);  
  mcp.writeRegister(MCP23017_GPPUB, 0xff);   
  // Input Polarity: low active = 1
  Serial.println("  - Input Polarity: low active");
  delay(100);
  mcp.writeRegister(MCP23017_IPOLA, 0xff);  
  mcp.writeRegister(MCP23017_IPOLB, 0xff);   
 
  #if DO_IRQ
    // no Mirror, Open-Drain, LOW-active
    Serial.println("    - IRQ: no Mirror, Open-Drain, LOW-active");        
    delay(100);
    mcp.setupInterrupts(0, 1, 0);          
    // Interrupt Mode: on-default / on-change
    Serial.println("    - IRQ Mode: Change");
    delay(100);
    mcp.writeRegister(MCP23017_INTCONA, 0x00);  // 0x00: Change
    mcp.writeRegister(MCP23017_INTCONB, 0x00);  // 0xff: Default      
    // Default Value 
    Serial.println("    - IRQ: Set Default Values");    
    delay(100);
    mcp.writeRegister(MCP23017_DEFVALA, 0x00);  // A
    mcp.writeRegister(MCP23017_DEFVALB, 0x00);  // B    
    // Interrupt Enable
    Serial.println("    - Enable Interrupts");    
    delay(100);
    mcp.writeRegister(MCP23017_GPINTENA, 0xff); // 1: Enable 
    mcp.writeRegister(MCP23017_GPINTENB, 0x00); // 0: Disable    
    // clearInterrupts
    Serial.println("    - clearInterrupts");
    delay(100);
    uint8_t i;
    i = mcp.readRegister(MCP23017_INTCAPA);
    i += mcp.readRegister(MCP23017_INTCAPB);        
  #endif // DO_IRQ 
}


/***********
 Setup
 ***********/
void setup() {        
  Serial.begin(115200);  
  Serial.println("");
  Serial.println("####################################");
  Serial.println("### Darios Homeautomation v2.0.0 ###");
  Serial.println("####################################");
  Serial.println("Init ...");
  delay(1000);

  // MCP23017
  Serial.println("- MCP23017 #1");
  setupInputMcp(mcp[0], 0);

  // Arduino IRQ
  #if DO_IRQ    
    Serial.print("- Arduino IRQ ...");
    pinMode(INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INT_PIN), iqrHandler, FALLING);
    Serial.println(" done.");    
  #endif // DO_IRQ
  delay(500);

  // Set I2C Speed
  Serial.print("- I2C: Set Speed to ");  
  Serial.print(I2CSPEED);
  Serial.print(" ...");
  delay(100);
  // Wire.setClock(I2CSPEED);
  Serial.println(" done.");
  delay(500);
 
  // Global vars
  Serial.print("- Global Vars ... ");
  delay(100);
  laststate = 0xff;
  lastReset = millis();
  lastPrint = millis();
  irqFlag = false;
  Serial.println("done.");
  delay(500);

  // init finished
  Serial.println("Init complete, starting Main-Loop"); 
  Serial.println("#################################");
  delay(1000);
}

/*******************
 *** Print State ***
 *******************
 prints:
  "Port[A]: -1----11 - 0x67"
 ***********/
void printState(uint8_t p, uint8_t s) {
  uint8_t i;
  // Print Label
  Serial.print("Port[");
  if (p==1) {
    Serial.print("A");
  } else if (p==2) {
    Serial.print("B");
  } else {
    Serial.print("undefined");
  } 
  Serial.print("]: ");
  // Print Bits  
  for (i=0; i<8; i++) {
    if ((s & (1<<i)) == 0) {
      Serial.print("-");
    } else {
      Serial.print("1");
    }
  }  
  Serial.print(" - 0x");
  // Print Value 
  Serial.println(s);
}

/**************************
 *** Print 16-Bit State ***
 **************************
 prints:
  "Port[A+B]: -1----11 -1----11 - 0x67"
 ***********/
void printStateAB(uint16_t s) {
  uint8_t i;
  // Print Label
  Serial.print("Port[A+B]: ");
  // Print Bits  
  for (i=0; i<16; i++) {
    if ((s & (1<<i)) == 0) {
      Serial.print("-");
    } else {
      Serial.print("1");
    }
    if (i==7) {
      Serial.print(" ");
    }
  }  
  Serial.print(" - 0x");
  // Print Value 
  Serial.println(s,HEX);
}

#if DO_SPEED
  /*****************
   *** Speedtest ***
   *****************/
  void SpeedTest() {
    if (millis() - lastPrint > SPEEDBEAT) {    
      uint32_t i;
      uint32_t startT;
      uint32_t endT;
      uint16_t us16bit;
      uint16_t us8bit;
      Serial.println("-------------------------------------");
      Serial.println("Speedtest started ...");
      // 8 Bit Mode
      Serial.println("1: 8 Bit Mode (read MCP-Registers seperate)");
      startT = millis();
      for (i=0; i<SPEEDRUNS; i++) {
        v8 = mcp.readGPIO(0);                // Read Port A + B
        v8 = mcp.readGPIO(1);                // Read Port A + B          
      }     
      endT = millis();    
      us8bit = (endT-startT) / SPEEDUSDIVISOR;
      Serial.print("   Started: ");     Serial.print(startT);  
      Serial.print(" - Finished: ");    Serial.print(endT);  
      Serial.print(" - Dauer: ");       Serial.println(endT-startT);              
      Serial.println("-------------------------------------");
      
      // 16 Bit Mode
      Serial.println("2: 16 Bit Mode (read both MCP-Registers in one call)");
      startT = millis();
      for (i=0; i<SPEEDRUNS; i++) {      
        v16 = mcp.readGPIOAB();                // Read Port A + B    
      } 
      endT = millis();    
      us16bit = (endT-startT) / SPEEDUSDIVISOR;
      Serial.print("   Started: ");     Serial.print(startT);  
      Serial.print(" - Finished: ");    Serial.print(endT);  
      Serial.print(" - Dauer: ");       Serial.println(endT-startT);  
      Serial.println("-------------------------------------");
      Serial.print("  I2C-Speed: ");  Serial.println(I2CSPEED);
      Serial.print("      8-Bit: ");  Serial.print(us8bit);   Serial.println("us per Sample");  
      Serial.print("     16-Bit: ");  Serial.print(us16bit);  Serial.println("us per Sample");  
      Serial.println("--------------------------------------------------------------------------");        
      lastPrint = millis();
    }
  }
#endif  // DO_SPEED

#if DO_HEARTBEAT
  /*******************
   *** Read Inputs ***
   *******************/
  void ReadInputs() {
    if (millis() - lastPrint > HEARTBEAT) {    
      lastPrint = millis();
      Serial.print("HeartBeat [");
      Serial.print(lastPrint);
      Serial.println("]");      
      // 16 Bit Read
      Serial.println("16Bit Read:");
      v16 = mcp.readGPIOAB();                // Read Port A + B    
      printStateAB(v16);      
      // 8 Bit Read
      Serial.println("8Bit Read:");
      v16 = mcp.readGPIO(0);                // Read Port A       
      v8  = mcp.readGPIO(1);                // Read Port B  
      v16 = v16 + (v8 << 8);
      printStateAB(v16);
      Serial.println("-------------------------------------");        
    } 
  } 
#endif // DO_HEARTBEAT


#if DO_IRQ
  /*******************
   *** Process IRQ ***
   *******************/
  void ProcessIrq() {
    uint8_t  i_pin;
    uint16_t i_port;    
    uint8_t intstate;

    // Print Value
    if (irqFlag) {
      Serial.println("Interrupt!");
      // Print INTCAP-Register
      i_pin = mcp[0].readRegister(MCP23017_INTCAPA);
      Serial.print("  - INTCAPA: ");
      Serial.println(i_pin);
      i_pin = mcp[0].readRegister(MCP23017_INTCAPB);
      Serial.print("  - INTCAPB: ");
      Serial.println(i_pin);
      // Print Portstate      
      i_port = mcp[0].readGPIOAB();  // Read Port A + B    
      Serial.print("  - ");
      printStateAB(i_port);
      Serial.println("-------------------------------------");
      irqFlag = false;    
    } 

    // Arduino IRQ-Pin changed        
    intstate = digitalRead(INT_PIN);
    if (laststate != intstate) {
      laststate = intstate;
      Serial.print("INT: ");
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
          Serial.println("Reset IRQ");                   
        } 
      }
    }
  } 
#endif // DO_IRQ

/***********
 Main Loop
 ***********/
void loop(){ 
  uint8_t  v16;
  uint8_t  v8;  

  #if DO_SPEED
  SpeedTest();
  #endif  
  
  #if DO_HEARTBEAT
    ReadInputs();
  #endif  

  #if DO_IRQ  
    ProcessIrq();
  #endif  


  // to surpress "unused Variable" warnings
  v16 = 0;
  v8 = 15;
  v16 = v8++;
  v8 = (v16 | 0xff);
} 