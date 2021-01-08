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
#define EVENT_NULL            (0<<5)   // not configured
#define EVENT_SPECIAL         (0<<5)   // followed Special Event ID
#define EVENT_ON              (1<<5)   // followed by Output which has to be switched on  [0x20]
#define EVENT_OFF             (2<<5)   // followed by Output which has to be switched off [0x40]
#define EVENT_TOGGLE          (3<<5)   // followed by Output which has to be toggled      [0x60]
#define EVENT_ROLLER_ACTION   (4<<5)   // followed by Roller MASK                         [0x80]  
#define EVENT_ROLLER_UP       (5<<5)   // followed by Roller MASK                         [0xA0] 
#define EVENT_ROLLER_DOWN     (6<<5)   // followed by Roller MASK                         [0xC0]        
#define EVENT_ROLLER_STOP     (7<<5)   // followed by Roller MASK                         [0xE0] 
// Special Commands 
#define CMD_SPEED             0x00     // Wait 0.N Seconds after each following Command (max 25.5s) - 2 Byte Command
#define CMD_WAIT              0x01     // Wait 0.N Seconds (max 25.5s) - 2 Byte Command
 

/********************************************************
 * Table Indicies for readFactoryDefaultTable()
 ********************************************************/
#define TABLE_INDEX_CLICK          0
#define TABLE_INDEX_CLICK_DOUBLE   1
#define TABLE_INDEX_CLICK_LONG     2
#define TABLE_INDEX_ROLLER         3


/********************************************************
 * Roller Action Types
 ********************************************************/
#define ROLL_STOP             0     // Stop 
#define ROLL_START_UP         1     // Start moving up
#define ROLL_START_DOWN       2     // Start moving down
#define ROLL_START_OPPOSITE   3     // Start moving opposite to last direction
#define ROLL_START_SAME       4     // Start moving same as last direction
#define ROLL_ACTION           5     // Roller Click State Machine (Start opposite - Stop - Start opposite - Stop...)
#define ROLL_TICK             6     // Poll Roller (to Stop a moving Roller after Time)
#define ROLLER_NC             0xff  // Roller not connected


#endif  //  _MYDEFINES_H_