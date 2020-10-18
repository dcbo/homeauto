/************************************************************
 * In this File everything which is related to the 
 * Funcional Design is defined:
 * - Name of the Buttons connected to the Input Ports
 * - Name of the Devices connected to the Output Ports
 * - Factory Default Settings
 * - Time Constants for Button State Machine
 ************************************************************
 * ToDo-List
 * - Special Events 
 *   - Scenes
 *   - Timers
 * - Roller Configuration
 *   - Time Constants for Rollers
 *   - Port Configuration
 ************************************************************/ 
#ifndef _MYSETTINGS_H_
#define _MYSETTINGS_H_

/********************************************************
 * Input (Button) Names
 ********************************************************/
#define in_3R1          IN_01    // Rollade Kinderzimmer Bett
#define in_3R2          IN_02    // Rollade Kinderzimmer Schrank
#define in_2R1          IN_03    // Rollade Schlafzimmer Yvonne
#define in_2R2          IN_04    // Rollade Schlafzimmer Dario

#define in_S1           IN_05    // Wohnzimmer 4er - 1
#define in_S2           IN_06    // Wohnzimmer 4er - 2
#define in_S3           IN_07    // Wohnzimmer 4er - 3
#define in_S4           IN_08    // Wohnzimmer 4er - 4

#define in_S5           IN_09    // Wohnzimmer Balkon oben
#define in_S6           IN_10    // Wohnzimmer Balkon unten

#define in_S7           IN_11    // Wohnzimmer Schlafzimmer oben
#define in_S8           IN_12    // Wohnzimmer Schlafzimmer unten

#define in_S9           IN_13    // Wohnzimmer Kinderzimmer oben
#define in_S10          IN_14    // Wohnzimmer Kinderzimmer unten

#define in_S11          IN_15    // Diele Wohnunseingang

#define in_2S1          IN_16    // Schlafzimmer
#define in_7S1          IN_17    // Küche
#define in_7S2          IN_18    // Vorratskammer

#define in_13S1         IN_19    // Bad oben
#define in_13S2         IN_20    // Bad unten

#define in_14S1         IN_21    // Gäste WC

#define in_3S2          IN_22    // Büro


/********************************************************
 * Output Names
 ********************************************************/
#define out_R1_up       OUT_01   // Roller 1 Up
#define out_R1_down     OUT_02   // Roller 1 Down
#define out_R2_up       OUT_03   // Roller 2 Up
#define out_R2_down     OUT_04   // Roller 2 Down
#define out_R3_up       OUT_05   // Roller 3 Up
#define out_R3_down     OUT_06   // Roller 3 Down
#define out_R4_up       OUT_07   // Roller 4 Up
#define out_R4_down     OUT_08   // Roller 4 Down

#define out_L1          OUT_09   // Licht Wohnzimmer 1
#define out_L2          OUT_10   // Licht Wohnzimmer 2
#define out_L3          OUT_11   // Licht Wohnzimmer 3
#define out_L4          OUT_12   // Licht Wohnzimmer Säule

#define out_L5          OUT_13   // Licht Diele

#define out_2L1         OUT_14   // Licht Schlafzimmer

#define out_3L1         OUT_15   // Licht Kinderzimmer Bettseite
#define out_3L2         OUT_16   // Licht Kinderzimmer Schrankseite

#define out_7L1         OUT_17   // Licht Küche
#define out_7L2         OUT_18   // Licht Vorratskammer
#define out_7D4         OUT_19   // Strom E-Box Küche

#define out_13L1        OUT_20   // Licht Bad Decke
#define out_13L2        OUT_21   // Licht Bad Spiegel
#define out_13L3        OUT_22   // Licht Bad Sternenhimmel

#define out_14L1        OUT_23   // Licht Gäste-WC Spiegel
#define out_14L2        OUT_24   // Licht Gäste-WC Decke
#define out_14M1        OUT_25   // Licht Gäste-WC Motor

#define out_6D1         OUT_26   // Steckdosen Balkon 

#define out_3D3         OUT_27   // Obere Steckdose zwischen 1. und 2. Balkontür
#define out_3D4         OUT_28   // Obere Steckdose zwischen 2. und 3. Balkontür
#define out_8D1         OUT_29   // Linke Steckdose hinter dem Backofen


/********************************************************
 * Special Actions
 *******************************************************/
#define SE_NONE          0         // Special Action: No Action
#define SE_3L1_3L2       1         // Special Action: Licht Kinderzimmer: Bettseite + Schrankseite
#define SE_CHRISTMAS     2         // Special Action: Christmas Lights (out_3D3 + out_3D4 + out_8D1)
#define SE_LEAVING       3         // Special Action: Leaving - Alle Lichter aus 
 

/********************************************************
 * Events
 ********************************************************
 * Main Config is done here. (Factory Defaults)
 * Values are written in Tables in EEPROM during boot.
 * ToDo:
 * - Implement Function to alter Configuration
 ******************************************************** 
 * EEPROM Format: (EEPROM Size is 1024 Byte)
 * - Click, Double-Click, Long-Click Events are 
 *   stored in three seperate tables.
 * - We have 32 Inputs, so each table contains 32 actions
 *   First Table-Entry corresponds to First Input
 * - Size of each Table Entry is 1 Byte
 *   - Upper three Bit ((value && 0xE0)>>6) 
 *     containing the Event Type EVENT_SPECIAL, EVENT_TOGGLE, 
 *     EVENT_ON, EVENT_OFF, EVENT_ROLLER_ACTION, EVENT_ROLLER_UP, 
 *     EVENT_ROLLER_DOWN, EVENT_ROLLER_STOP
 *   - Lower six Bit (value && 0x3F) 
 *     containing the Parameter for the Event. In case of 
 *     - EVENT_TOGGLE, EVENT_ON and EVENT_OFF -> Output Pin
 *     - EVENT_SPECIAL -> # of Special Event [TBD]
 * - EVENT_SPECIAL 0 defines no Action
 ********************************************************
 * For all three Tables 96 Byte of EEPROM is needed.
 * We use the following EEPOM Adresses:
 * - 0x00: Click on Input Pin 1
 * - 0x20: Double Click on Input Pin 1
 * - 0x40: Long Click on Input Pin 1
 * - 0x5f: Long Click on Input Pin 32
 ********************************************************
 * In Order to use the Names for Inputs and Outputs the
 * following Field is stored in FLASH as factory default.
 * During Boot it is used to generate the EEPROM Table
 * - One Field is used each Button Click Types and 
 * - two Byte are used for each entry:
 *   - Byte 1: Number of INPUT-Pin: e.g.: in_S1 
 *   - Byte 2: Action = EVENT + Parameter
 *     - Parameter:
 *       - if EVENT_ON, EVENT_OFF, EVENT_TOGGLE: Parameter = Output-Pin
 *       - if EVENT_ROLLER_...: Parameter = Output-Pin
 *       - if EVENT_SPECIAL: Parameter = Special Event 
 *                           if Parameter = 0: No Command defined
 ********************************************************/

// BUTTON_CLICK - Events
static const uint8_t FactoryDefaultClickTable[][2] PROGMEM = {            
    {in_3R1,  EVENT_ROLLER_ACTION + MASK_1},      // Rollade Kinderzimmer Bett      -> Roller-Action (Bett)
    {in_3R2,  EVENT_ROLLER_ACTION + MASK_2},      // Rollade Kinderzimmer Schrank   -> Roller-Action (Schrank)
    {in_2R1,  EVENT_ROLLER_ACTION + MASK_3},      // Rollade Schlafzimmer Yvonne    -> Roller-Action (Yvonne)
    {in_2R2,  EVENT_ROLLER_ACTION + MASK_4},      // Rollade Schlafzimmer Dario     -> Roller-Action (Dario) 
    {in_S1,   EVENT_TOGGLE + out_L1},            // Wohnzimmer 4er - 1             -> Licht Wohnzimmer 1
    {in_S2,   EVENT_TOGGLE + out_L2},            // Wohnzimmer 4er - 2             -> Licht Wohnzimmer 2
    {in_S3,   EVENT_TOGGLE + out_L3},            // Wohnzimmer 4er - 3             -> Licht Wohnzimmer 3
    {in_S4,   EVENT_TOGGLE + out_L5},            // Wohnzimmer 4er - 4             -> Licht Diele
    {in_S5,   EVENT_TOGGLE + out_L3},            // Wohnzimmer Balkon oben         -> Licht Wohnzimmer 3
    {in_S6,   EVENT_TOGGLE + out_7L1},           // Wohnzimmer Balkon unten        -> Licht Küche
    {in_S7,   EVENT_TOGGLE + out_L1},            // Wohnzimmer Schlafzimmer oben   -> Licht Wohnzimmer 1
    {in_S8,   EVENT_TOGGLE + out_L3},            // Wohnzimmer Schlafzimmer unten  -> Licht Wohnzimmer 3
    {in_S9,   EVENT_TOGGLE + out_L1},            // Wohnzimmer Kinderzimmer oben   -> Licht Wohnzimmer 1
    {in_S10,  EVENT_TOGGLE + out_L3},            // Wohnzimmer Kinderzimmer unten  -> Licht Wohnzimmer 3
    {in_S11,  EVENT_TOGGLE + out_L5},            // Diele Wohnunseingang           -> Licht Diele
    {in_2S1,  EVENT_TOGGLE + out_2L1},           // Schlafzimmer                   -> Licht Schlafzimmer
    {in_7S1,  EVENT_TOGGLE + out_7L1},           // Küche                          -> Licht Küche
    {in_7S2,  EVENT_TOGGLE + out_7L2},           // Vorratskammer                  -> Licht Vorratskammer
    {in_13S1, EVENT_TOGGLE + out_13L2},          // Bad oben                       -> Licht Bad Spiegel
    {in_13S2, EVENT_TOGGLE + out_13L1},          // Bad unten                      -> Licht Bad Decke
    {in_14S1, EVENT_TOGGLE + out_14L1},          // Gäste-WC                       -> Licht Gäste-WC: Decke
    {in_3S2,  EVENT_SPECIAL + SE_3L1_3L2}        // Kinderzimmer                   -> Licht Kinderzimmer: (Bett + Schrank)
};


// BUTTON_DOUBLE_CLICK - Events
static const uint8_t FactoryDefaultClickDoubleTable[][3] PROGMEM = {        
    {in_3R1,  EVENT_ROLLER_ACTION + MASK_1 + MASK_2}, // Rollade Kinderzimmer Bett      -> Roller-Action (1 + 2)
    {in_3R2,  EVENT_ROLLER_ACTION + MASK_1 + MASK_2}, // Rollade Kinderzimmer Schrank   -> Roller-Action (1 + 2)
    {in_2R1,  EVENT_ROLLER_ACTION + MASK_3 + MASK_4}, // Rollade Schlafzimmer Yvonne    -> Roller-Action (3 + 4) 
    {in_2R2,  EVENT_ROLLER_ACTION + MASK_3 + MASK_4}, // Rollade Schlafzimmer Dario     -> Roller-Action (3 + 4)
    {in_S1,   EVENT_SPECIAL},                         // Wohnzimmer 4er - 1             -> [     ]
    {in_S2,   EVENT_SPECIAL},                         // Wohnzimmer 4er - 2             -> [     ]
    {in_S3,   EVENT_SPECIAL},                         // Wohnzimmer 4er - 3             -> [     ]
    {in_S4,   EVENT_SPECIAL},                         // Wohnzimmer 4er - 4             -> [     ]
    {in_S5,   EVENT_SPECIAL},                         // Wohnzimmer Balkon oben         -> [     ] 
    {in_S6,   EVENT_SPECIAL},                         // Wohnzimmer Balkon unten        -> [     ]
    {in_S7,   EVENT_ROLLER_UP + MASK_3 + MASK_4},     // Wohnzimmer Schlafzimmer oben   -> Roller-Up   (3 + 4)
    {in_S8,   EVENT_ROLLER_DOWN + MASK_3 + MASK_4},   // Wohnzimmer Schlafzimmer unten  -> Roller-Down (3 + 4)
    {in_S9,   EVENT_ROLLER_UP + MASK_1 + MASK_2},     // Wohnzimmer Kinderzimmer oben   -> Roller-Up   (1 + 2)
    {in_S10,  EVENT_ROLLER_DOWN + MASK_1 + MASK_2},   // Wohnzimmer Kinderzimmer unten  -> Roller-Down (1 + 2) 
    {in_S11,  EVENT_SPECIAL + SE_LEAVING},            // Diele Wohnunseingang           -> Leaving (alles aus, bis auf ...)
    {in_2S1,  EVENT_SPECIAL},                         // Schlafzimmer                   -> [      ]
    {in_7S1,  EVENT_TOGGLE  + out_L3},                // Küche                          -> Licht Licht Wohnzimmer 3
    {in_7S2,  EVENT_SPECIAL},                         // Vorratskammer                  -> [      ]
    {in_13S1, EVENT_TOGGLE + out_13L3},               // Bad oben                       -> Licht Bad Sternenhimmel    
    {in_13S2, EVENT_TOGGLE + out_13L3},               // Bad unten                      -> Licht Bad Sternenhimmel
    {in_14S1, EVENT_TOGGLE + out_14L2},               // Gäste-WC                       -> Licht Gäste-WC Decke
    {in_3S2,  EVENT_TOGGLE + out_3L2}                 // Kinderzimmer                   -> Licht Kinderzimmer Schrankseite
};

// BUTTON_LONG_CLICK - Events
static const uint8_t FactoryDefaultClickLongTable[][3] PROGMEM = {    
    {in_3R1,  EVENT_ROLLER_STOP + MASK_1 + MASK_2},   // Rollade Kinderzimmer Bett      -> [Roller-Stop (1 + 2)]
    {in_3R2,  EVENT_ROLLER_STOP + MASK_1 + MASK_2},   // Rollade Kinderzimmer Schrank   -> [Roller-Stop (1 + 2)]
    {in_2R1,  EVENT_ROLLER_STOP + MASK_3 + MASK_4},   // Rollade Schlafzimmer Yvonne    -> [Roller-Stop (3 + 4)]
    {in_2R2,  EVENT_ROLLER_STOP + MASK_3 + MASK_4},   // Rollade Schlafzimmer Dario     -> [Roller-Stop (3 + 4)]
    {in_S1,   EVENT_SPECIAL},                         // Wohnzimmer 4er - 1             -> [     ]
    {in_S2,   EVENT_SPECIAL},                         // Wohnzimmer 4er - 2             -> [     ]
    {in_S3,   EVENT_SPECIAL},                         // Wohnzimmer 4er - 3             -> [     ]
    {in_S4,   EVENT_SPECIAL},                         // Wohnzimmer 4er - 4             -> [     ]
    {in_S5,   EVENT_TOGGLE + out_6D1},                // Wohnzimmer Balkon oben         -> Balkon 
    {in_S6,   EVENT_SPECIAL + SE_CHRISTMAS},          // Wohnzimmer Balkon unten        -> (out_3D3 + out_3D4 + out_8D1)
    {in_S7,   EVENT_TOGGLE + out_6D1},                // Wohnzimmer Schlafzimmer oben   -> Balkon
    {in_S8,   EVENT_SPECIAL + SE_CHRISTMAS},          // Wohnzimmer Schlafzimmer unten  -> (out_3D3 + out_3D4 + out_8D1)
    {in_S9,   EVENT_TOGGLE + out_6D1},                // Wohnzimmer Kinderzimmer oben   -> Balkon
    {in_S10,  EVENT_SPECIAL + SE_CHRISTMAS},          // Wohnzimmer Kinderzimmer unten  -> (out_3D3 + out_3D4 + out_8D1)
    {in_S11,  EVENT_SPECIAL},                         // Diele Wohnunseingang           -> [     ]
    {in_2S1,  EVENT_SPECIAL},                         // Schlafzimmer                   -> [     ]
    {in_7S1,  EVENT_SPECIAL},                         // Küche                          -> [     ]
    {in_7S2,  EVENT_SPECIAL},                         // Vorratskammer                  -> [     ]
    {in_13S1, EVENT_SPECIAL},                         // Bad oben                       -> [     ]
    {in_13S2, EVENT_SPECIAL},                         // Bad unten                      -> [     ]
    {in_14S1, EVENT_SPECIAL},                         // Gäste-WC                       -> [     ]
    {in_3S2,  EVENT_SPECIAL}                          // Kinderzimmer                   -> [     ]
};

/********************************************************
 * Spacial Events Table
 ********************************************************
 * Special Event Table is written to EEPROM during boot.
 ******************************************************** 
 * EEPROM Format: 
 * - Special Events are stored in two tables
 * - First Table is the Special Events Pointer Table.
 * - Starting at EE_OFFSET_SPECIAL_EVENT = 0x70
 *   - 0x070: Number of Special Events
 *   - 0x071, 0x072: Adress of 1st Special Event =SE1
 *     0x073, 0x074: Adress of 2nd Special Event
 *     - 0x070+(n*2),     0x071+(n*2)     : Adress Last Element 
 *     - 0x070+((n+1)*2), 0x071+((n+1)*2) : End-Adress Last Element 
 * - Each Special Event goes from [Address of Element] to [Address of next Element]-1
 *   -eg: SE#1 from 0xSE1 to 0xSE2-1
 * - All Commands of a Special Event are handled one after another.
 * - Ther are One-Byte and Multiple-Byte Commands:
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
 ********************************************************
 * In Order to use the Names for Inputs and Outputs the
 * following Field is stored in FLASH as factory default.
 * During Boot it is used to generate the EEPROM Table
 ********************************************************/
// Special Events



static const uint8_t FactoryDefaultSpecialEventsTable[] PROGMEM = {    
    // # of Special Events:
    3,
        // Special Event SE_3L1_3L2:
        // 2  1-Byte Events 
        //    Toggle 3L1
        //    Toggle 3L2
        2,                            
            EVENT_TOGGLE + out_3L1,       
            EVENT_TOGGLE + out_3L1,       
        // Special Event SE_CHRISTMAS
        // 3 1-Byte Events 
        // Toggle 3D3
        // Toggle 3D4
        // Toggle 8D1
        3,                            
            EVENT_TOGGLE + out_3D3,       
            EVENT_TOGGLE + out_3D4,       
            EVENT_TOGGLE + out_8D1,       
        // Special Event SE_LEAVING
        // 17 1-Byte Events, 2 2-Byte Events
        //    On L5
        //    Wait 2s
        //    Licht Wohnzimmer 1
        //    Licht Wohnzimmer 2
        //    Licht Wohnzimmer 3
        //    Licht Wohnzimmer Säule
        //    Licht Schlafzimmer
        //    Licht Kinderzimmer Bettseite
        //    Licht Kinderzimmer Schrankseite
        //    Licht Küche
        //    Licht Vorratskammer
        //    Licht Bad Decke
        //    Licht Bad Spiegel
        //    Licht Bad Sternenhimmel
        //    Licht Gäste-WC Spiegel
        //    Licht Gäste-WC Decke
        //    Licht Gäste-WC Motor
        //    Wait 2s
        //    Off L5
        21,                           
            EVENT_ON + out_L5,
            CMD_WAIT, 20,     
            EVENT_OFF + out_L1,
            EVENT_OFF + out_L2,
            EVENT_OFF + out_L3,
            EVENT_OFF + out_L4,
            EVENT_OFF + out_2L1,
            EVENT_OFF + out_3L1,
            EVENT_OFF + out_3L2,
            EVENT_OFF + out_7L1,
            EVENT_OFF + out_7L2,
            EVENT_OFF + out_13L1,
            EVENT_OFF + out_13L2,
            EVENT_OFF + out_13L3,
            EVENT_OFF + out_14L1,
            EVENT_OFF + out_14L2,
            EVENT_OFF + out_14M1,
            CMD_WAIT, 20,        
            EVENT_OFF + out_L5   
};



/********************************************************
 * Roller: Port Configuration and Time Constants 
 ********************************************************
 * - Rollers are attached to two OUTPUT Ports one for each
 *   motor direction. The Port for Down-Direction must be 
 *   the Port following the Port which drives the motor 
 *   up. e.g: 
 *   - UP Port 7, DOWN Port 8 * 
 * - For each Roller the Time to move completely up/down
 *   has to be configured in Half Seconds [500ms]
 * - A maximum of 4 Rollers is supported
 ******************************************************** 
 * EEPROM Format: 
 * - Roller Table starts at: EE_OFFSET_ROLLER
 *   EE_OFFSET_ROLL_NUM
 * - Pointer to Roller Table ist stored at EE_OFFSET_ROLL_ADR  
 * - Four values for each Roller are stored in the Table
 *   - Port #:  Output of Motor wich drives the Roller up 
 *              use 0xff (ROLLER_NC) is no Roller connected
 *   - Up-Time:      Half-Seconds for complete travel up
 *   - Down-Time:    Half-Seconds for complete travel down
 *   - Default-Time: Half-Seconds to travel down to default Close Position
 * - Four Rollers must be configured (16 Byte)
 *   - If a Roller is not connected, use ROLLER_NC (0xff) as Port #
 ********************************************************/

static const uint8_t FactoryDefaultRollerTable[][4] PROGMEM = {    
    {out_R1_up, 46, 45, 44},     //  Roller 1:  Kinderzimmer Bett:     Up: 23s, Down 22.5s, Close: 22s
    {out_R2_up, 46, 45, 36},     //  Roller 2:  Kinderzimmer Schrank:  Up: 23s, Down 22.5s, Close: 18s
    {out_R3_up, 46, 45, 32},     //  Roller 3:  Schlafzimmer Yvonne:   Up: 23s, Down 22.5s, Close: 16s
    {out_R4_up, 46, 45, 40}      //  Roller 4:  Schlafzimmer Dario:    Up: 23s, Down 22.5s, Close: 20s
};  


/********************************************************
 * Time Constants for Button State Machine
 ********************************************************/
#define BUTTON_T0            20  // <T0= No Klick (Noise)  20ms (T0-1)*10ms (max   30ms)
#define BUTTON_T1          1000  // >T1= Long Klick       950ms (T1-1)*10ms (max 1000ms)
#define BUTTON_T2           200  // <T2= Double Klick     190ms (T2-1)*10ms (max  200ms)
#define BUTTON_SCANINT       10  // [ms] Scan interval after IRQ occured
#define BUTTON_SCANTIME    1000  // [ms] Stop scanning every BUTTON_SCANINT after this time 


/********************************************************
 * Timers
 ********************************************************/
#define MAX_TIMER             4
#define tm_1_time             0
#define tm_1_event
#define tm_2_time             0
#define tm_2_event
#define tm_3_time             0
#define tm_3_event
#define tm_4_time             0
#define tm_4_event


#endif  // _MYSETTINGS_H_