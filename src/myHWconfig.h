/************************************************************
 * In this File everything which is related to the 
 * Hardware Design is defined:
 * - Number of MCP-Chips
 * - EEPROM Offsets
 * - Mappings of MCP-Ports to Terminals 
 * - Speed of I2C Bus
 * - MCP23017 Interupt Pin to Arduino Pin (only Input MCPs)
 * - MCP23017 Reset Pin to Arduino Pin
 ************************************************************/ 
#ifndef _MYHWCONFIG_H_
#define _MYHWCONFIG_H_


/************************************************************
 * Number of MCP Chips
 * - Input Chips must coded with the lower I2C-Adress starting from 0
 ************************************************************/ 
#define MCP_IN_NUM            2                          // # of Input Chips starting from adr 0
#define MCP_OUT_NUM           2                          // # of Output Chips 
#define MCP_IN_PINS           MCP_IN_NUM*16              // # of Input Pins
#define MCP_OUT_PINS          MCP_OUT_NUM*16             // # of Output Pins
#define MCP_NUM               MCP_IN_NUM + MCP_OUT_NUM   // # of total MCP Chips


/********************************************************
 * EEPROM OFFSETS (see Events)
 ******************************************************** 
 * EEPROM Format: 
 * 0x000-0x01F: Click Table                            [EE_OFFSET_CLICK]
 * 0x010-0x03F: Double-Click Table                     [EE_OFFSET_CLICK_DOUBLE]
 * 0x040-0x05F: Long-Click Table                       [EE_OFFSET_CLICK_LONG]
 * 0x060      : Number of Rollers                      [EE_OFFSET_ROLL_NUM]
 * 0x061      : Adress of Roller-Config Table [RRRR]   [EE_OFFSET_ROLL_ADR]
 * 0x062      : Number of Special Events               [EE_OFFSET_SPECIAL_EVENT_NUM]
 * 0x063      : Adress of Special Events-Table [SSSS]  [EE_OFFSET_SPECIAL_EVENT_ADR]
 *********************************************************
 * Roller-Config Table:                                [EE_OFFSET_BEGIN_VARSPACE]
 * [RRRR]     :  Two values for each Roller            
 *              - Time to move completely up [* 500ms]
 *              - Time to move completely down [* 500ms]
 *********************************************************
 * Special Events-Table:
 * [SSSS]     : Address of Special Event #0  [SE#0]
 * [SSSS] +1  : Address of Special Event #1  [SE#1]
 *   ...
 * [SE#0]     : Special Event #0
 * [SE#0] +1
 * [SE#0] +2
 * [SE#0] +3
 *   ...
 * [SE#1]     : Special Event #1
 * [SE#1] +1
 * [SE#1] +2 
 *   ...
 ********************************************************/
// Click Table: 32 Byte (Number of Input Pins)
#define EE_OFFSET_CLICK          0
// Double-Click Table: 32 Byte (Number of Input Pins)
#define EE_OFFSET_CLICK_DOUBLE       EE_OFFSET_CLICK + (MCP_IN_NUM * 16)
// Long-Click Table: 32 Byte (Number of Input Pins)
#define EE_OFFSET_CLICK_LONG         EE_OFFSET_CLICK_DOUBLE + (MCP_IN_NUM * 16)
// Number of Rollers
#define EE_OFFSET_ROLL_NUM           EE_OFFSET_CLICK_DOUBLE + (MCP_IN_NUM * 16)
// Address of Pointer to first Element of Roller Table 
#define EE_OFFSET_ROLL_ADR           EE_OFFSET_ROLL_NUM + 1
// Number of Special Events
#define EE_OFFSET_SPECIAL_EVENT_NUM  EE_OFFSET_ROLL_ADR + 1
// Address of Pointer to first Element of Special Events Table
#define EE_OFFSET_SPECIAL_EVENT_ADR  EE_OFFSET_SPECIAL_EVENT_NUM + 1
// Address of first Element in Variable EEPROM Space = first Element of Roller Table 
#define EE_OFFSET_BEGIN_VARSPACE     EE_OFFSET_SPECIAL_EVENT_ADR + 1


/********************************************************
 * MCP - Input Port Mappings
 ********************************************************
 *  U | I2C  |Mode | Port A           | Port B
 * ---|------+-----+------------------+------------------
 *  0 | 0x20 |  In | Input-Pin 09-16  | Input-Pin 01-08
 *  1 | 0x21 |  In | Input-Pin 24-32  | Input-Pin 17-24
 ********************************************************/
// MCP0 = left, adr 0x20, Port B
#define IN_01          8  // Input Pin  1: 0x20 B0
#define IN_02          9  // Input Pin  2: 0x20 B1
#define IN_03         10  // Input Pin  3: 0x20 B2
#define IN_04         11  // Input Pin  4: 0x20 B3
#define IN_05         12  // Input Pin  5: 0x20 B4
#define IN_06         13  // Input Pin  6: 0x20 B5
#define IN_07         14  // Input Pin  7: 0x20 B6
#define IN_08         15  // Input Pin  8: 0x20 B7
// MCP0 = left, adr 0x20, Port A
#define IN_09          0  // Input Pin  9: 0x20 A0
#define IN_10          1  // Input Pin 10: 0x20 A1
#define IN_11          2  // Input Pin 11: 0x20 A2
#define IN_12          3  // Input Pin 12: 0x20 A3
#define IN_13          4  // Input Pin 13: 0x20 A4
#define IN_14          5  // Input Pin 14: 0x20 A5
#define IN_15          6  // Input Pin 15: 0x20 A6
#define IN_16          7  // Input Pin 16: 0x20 A7
// MCP0 = right, adr 0x21, Port B
#define IN_17         24  // Input Pin 17: 0x20 B0
#define IN_18         25  // Input Pin 18: 0x20 B1
#define IN_19         26  // Input Pin 19: 0x20 B2
#define IN_20         27  // Input Pin 20: 0x20 B3
#define IN_21         28  // Input Pin 31: 0x20 B4
#define IN_22         29  // Input Pin 22: 0x20 B5
#define IN_23         30  // Input Pin 23: 0x20 B6
#define IN_24         31  // Input Pin 24: 0x20 B7
// MCP0 = right, adr 0x21, Port A
#define IN_25         16  // Input Pin 25: 0x20 A0
#define IN_26         17  // Input Pin 26: 0x20 A1
#define IN_27         18  // Input Pin 27: 0x20 A2
#define IN_28         19  // Input Pin 28: 0x20 A3
#define IN_29         20  // Input Pin 29: 0x20 A4
#define IN_30         21  // Input Pin 30: 0x20 A5
#define IN_31         22  // Input Pin 31: 0x20 A6
#define IN_32         23  // Input Pin 32: 0x20 A7


/********************************************************
 * MCP - Output Port Mappings
 ********************************************************
 *  U | I2C  |Mode | Port A           | Port B
 * ---|------+-----+------------------+------------------
 *  2 | 0x22 | Out | Output-Pin 08-01 | Output-Pin 16-09
 *  3 | 0x23 | Out | Output-Pin 24-17 | Output-Pin 32-25 
*********************************************************/
// MCP2 = left, adr 0x22, Port A
#define OUT_01        15  // Output Pin  1: 0x22 A7
#define OUT_02        14  // Output Pin  2: 0x22 A6
#define OUT_03        13  // Output Pin  3: 0x22 A5
#define OUT_04        12  // Output Pin  4: 0x22 A4
#define OUT_05        11  // Output Pin  5: 0x22 A3
#define OUT_06        10  // Output Pin  6: 0x22 A2
#define OUT_07         9  // Output Pin  7: 0x22 A1
#define OUT_08         8  // Output Pin  8: 0x22 A0
// MCP2 = left, adr 0x22, Port B
#define OUT_09         0  // Output Pin  9: 0x22 B7
#define OUT_10         1  // Output Pin 10: 0x22 B6
#define OUT_11         2  // Output Pin 11: 0x22 B5
#define OUT_12         3  // Output Pin 12: 0x22 B4
#define OUT_13         4  // Output Pin 13: 0x22 B3
#define OUT_14         5  // Output Pin 14: 0x22 B2
#define OUT_15         6  // Output Pin 15: 0x22 B1
#define OUT_16         7  // Output Pin 16: 0x22 B0
// MCP3 = right, adr 0x23, Port A
#define OUT_17        23  // Output Pin 17: 0x20 A7
#define OUT_18        22  // Output Pin 18: 0x20 A6
#define OUT_19        21  // Output Pin 19: 0x20 A5
#define OUT_20        20  // Output Pin 20: 0x20 A4
#define OUT_21        19  // Output Pin 31: 0x20 A3
#define OUT_22        18  // Output Pin 22: 0x20 A2
#define OUT_23        17  // Output Pin 23: 0x20 A1
#define OUT_24        16  // Output Pin 24: 0x20 A0
// MCP3 = right, adr 0x23, Port A
#define OUT_25        31  // Output Pin 25: 0x20 B7
#define OUT_26        30  // Output Pin 26: 0x20 B6
#define OUT_27        29  // Output Pin 27: 0x20 B5
#define OUT_28        28  // Output Pin 28: 0x20 B4
#define OUT_29        27  // Output Pin 29: 0x20 B3
#define OUT_30        26  // Output Pin 30: 0x20 B2
#define OUT_31        25  // Output Pin 31: 0x20 B1
#define OUT_32        24  // Output Pin 32: 0x20 B0


/************************************************************
 * I2C to 800kHz -> 127us to read all 16 GPIOs from one MCP23017
 ************************************************************/ 
#define I2CSPEED         800000  


/************************************************************
 * Interupt Pin (comes from Input-MCP23017s)
 ************************************************************/ 
#define INT_PIN               2  


/************************************************************
 * MCP Reset Pin (goes to all MCP23017-/Reset)
 ************************************************************/ 
#define MCP_RST_PIN           7


#endif // _MYHWCONFIG_H_