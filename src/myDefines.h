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
#ifndef _MYDEFINES_H_
#define _MYDEFINES_H_


/********************************************************
 * Button Click Types
 ********************************************************/
#define BUTTON_CLICK          0
#define BUTTON_CLICK_DOUBLE   1
#define BUTTON_CLICK_LONG     2

/********************************************************
 * Event Types
 ********************************************************/
#define EVENT_SPECIAL         0   // followed Special Event ID
#define EVENT_ON              1   // followed by Output which has to be toggled
#define EVENT_OFF             2   // followed by Output which has to be toggled
#define EVENT_TOGGLE          3   // followed by Output which has to be toggled

/********************************************************
 * Table Indicies for ReadFactoryDefaultTable()
 ********************************************************/
#define TABLE_INDEX_CLICK          0
#define TABLE_INDEX_CLICK_DOUBLE   1
#define TABLE_INDEX_CLICK_LONG     2
#define TABLE_INDEX_ROLLER         3


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


#endif  //  _MYDEFINES_H_