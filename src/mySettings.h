#ifndef _MYSETTINGS_H_
#define _MYSETTINGS_H_


/********************************************************
 * Input (Button) Names
 ********************************************************/
#define in_3R1          IN_01    // Rollade Büro rechts
#define in_3R2          IN_02    // Rollade Büro links
#define in_2R1          IN_03    // Rollade Schlafzimmer rechts
#define in_2R2          IN_04    // Rollade Schlafzimmer links

#define in_S1           IN_05    // Wohnzimmer 4er - 1
#define in_S2           IN_06    // Wohnzimmer 4er - 2
#define in_S3           IN_07    // Wohnzimmer 4er - 3
#define in_S4           IN_08    // Wohnzimmer 4er - 4

#define in_S5           IN_09    // Wohnzimmer Balkon oben
#define in_S6           IN_10    // Wohnzimmer Balkon unten

#define in_S7           IN_11    // Wohnzimmer Schlafzimmer oben
#define in_S8           IN_12    // Wohnzimmer Schlafzimmer unten

#define in_S9           IN_13    // Wohnzimmer Büro oben
#define in_S10          IN_14    // Wohnzimmer Büro unten

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

#define out_3L1         OUT_15   // Licht Charlotte Bettseite
#define out_3L2         OUT_16   // Licht Charlotte Schrankseite

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
 * Roller Action Types
 ********************************************************/
#define ROLL_STOP             0
#define ROLL_START_UP         1
#define ROLL_START_DOWN       2
#define ROLL_START_OPPOSITE   3
#define ROLL_START_SAME       4
#define ROLL_CLICK            5
#define ROLL_TICK             6

/********************************************************
 * Time Constants for Rollers 
 ********************************************************/
#define ROLL_num         4       // Number of Rolles
#define ROLL_R1_maxup    23000   // Time to open [ms]
#define ROLL_R1_maxdown  23000   // Time to close [ms]
#define ROLL_R2_maxup    23000   // Time to open [ms]
#define ROLL_R2_maxdown  23000   // Time to close [ms]
#define ROLL_R3_maxup    23000   // Time to open [ms]
#define ROLL_R3_maxdown  23000   // Time to close [ms]
#define ROLL_R4_maxup    23000   // Time to open [ms]
#define ROLL_R4_maxdown  23000   // Time to close [ms]

/********************************************************
 * Time Constants for Button State Machine
 ********************************************************/
#define BUTTON_T0            20  // <T0= No Klick (Noise)  20ms (T0-1)*10ms (max   30ms)
#define BUTTON_T1          1000  // >T1= Long Klick       950ms (T1-1)*10ms (max 1000ms)
#define BUTTON_T2           200  // <T2= Double Klick     190ms (T2-1)*10ms (max  200ms)
#define BUTTON_SCANINT       10  // [ms] Scan interval after IRQ occured
#define BUTTON_SCANTIME    1000  // [ms] Stop scanning every BUTTON_SCANINT after this time 



/********************************************************
 * Scenes
 ********************************************************/
#define SCENE_ALL_OFF         0  // Turn everything OFF
#define SCENE_ALL_ON          1  // Turn everything ON
#define SCENE_LEAVING         2  // Leaving the House
#define SCENE_BED             3  // Scene Bett: alles aus bis auf Balkonlicht
#define SCENE_BED_delay       4  // Scene Bett Delay: alles aus bis auf Balkonlicht, Schlafzimmer 30s an

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

/********************************************************
 * Event Types
 ********************************************************/
#define EVENT_SPECIAL         0   // followed Special Event ID
#define EVENT_ON              1   // followed by Output which has to be toggled
#define EVENT_OFF             2   // followed by Output which has to be toggled
#define EVENT_TOGGLE          3   // followed by Output which has to be toggled


/********************************************************
 * Button Click Types
 ********************************************************/
#define BUTTON_CLICK          0
#define BUTTON_CLICK_DOUBLE   1
#define BUTTON_CLICK_LONG     2


/********************************************************
 * EEPROM OFFSETS (see Events)
 ********************************************************/
#define EEPROM_OFFSET_CLICK          0
#define EEPROM_OFFSET_CLICK_DOUBLE   EEPROM_OFFSET_CLICK + (MCP_IN_NUM * 16)
#define EEPROM_OFFSET_CLICK_LONG     EEPROM_OFFSET_CLICK_DOUBLE + (MCP_IN_NUM * 16)
#define EEPROM_OFFSET_CLICK_LONG_END EEPROM_OFFSET_CLICK_DOUBLE + (MCP_IN_NUM * 16)
#define EEPROM_OFFSET_FIRST_FREE     EEPROM_OFFSET_CLICK_LONG_END


/********************************************************
 * Events
 ********************************************************
 * Main Config is done here. (factory DEFAULTS)
 * Values are written in Tables in EEPROM during boot.
 * ToDo:
 * - Implement Function to alter Configuration
 ******************************************************** 
 * Format: (EEPROM Size is 1024 Byte)
 * - Click, Double-Click, Long-Click Events are 
 *   stored in three seperate tables.
 * - We have 32 Inputs, so each table contains 32 actions
 *   First Table-Entry correspond to first Input
 * - Size of each Table Entry is 1 Byte
 *   - Upper two Bit ((value && 0xC0)>>6) 
 *     containing the Event Type EVENT_TOGGLE, EVENT_ON,
 *     EVENT_OFF, EVENT_SPECIAL
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
 * - three byte are used for each entry:
 *   - Number of INPUT-Pin: e.g.: in_S1 
 *   - Type of Event--> EVENT_SPECIAL, EVENT_ON, EVENT_OFF, EVENT_TOGGLE
 *   - Number of OUTPUT-Pin (or Special Event Parameter): e.g.: out_L1 
 ********************************************************/

// BUTTON_CLICK - Events
static const uint8_t FactoryDefaultClickTable[][3] PROGMEM = {    
    {1, EVENT_ON, 5} ,
    {2, EVENT_ON, 5} ,
    {3, EVENT_ON, 5} ,
    {4, EVENT_ON, 5} ,
    {in_S1, EVENT_TOGGLE, out_L1} ,  //  S1 -> TOGGLE L1
    {in_S2, EVENT_TOGGLE, out_L2} ,  //  S2 -> TOGGLE L2
    {in_S3, EVENT_TOGGLE, out_L3} ,  //  S3 -> TOGGLE L3
    {in_S4, EVENT_TOGGLE, out_L5}    //  S4 -> TOGGLE L5
};

// BUTTON_DOUBLE_CLICK - Events
static const uint8_t FactoryDefaultDoubleClickTable[][3] PROGMEM = {    
    {in_S1, EVENT_TOGGLE, out_L2} ,  //  S1 -> TOGGLE L2  For Test Only
    {in_S2, EVENT_TOGGLE, out_L3} ,  //  S2 -> TOGGLE L3  For Test Only
    {in_S3, EVENT_TOGGLE, out_L5} ,  //  S3 -> TOGGLE L5  For Test Only
    {in_S4, EVENT_TOGGLE, out_L1}    //  S4 -> TOGGLE L1  For Test Only
};

// BUTTON_LONG_CLICK - Events
static const uint8_t FactoryDefaultLongClickTable[][3] PROGMEM = {    
    {in_S1, EVENT_OFF, out_L1} ,     //  S1 -> OFF L1
    {in_S2, EVENT_OFF, out_L2} ,     //  S2 -> OFF L2
    {in_S3, EVENT_OFF, out_L3} ,     //  S3 -> OFF L3
    {in_S4, EVENT_OFF, out_L5}       //  S4 -> OFF L5
};




#endif  // _MYSETTINGS_H_