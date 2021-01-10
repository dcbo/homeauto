/*!
 * @file debugOptions.h
 */
#ifndef _DEBUGOPTIONS_H_
#define _DEBUGOPTIONS_H_

/************************************************************
 * Debugging Config
 ************************************************************/ 
#define DEBUG                 1  // Debug main 
#define DEBUG_ERROR           1  // Error Messages
#define DEBUG_EE_INIT         0  // Debug EEPROM Init [1142 Byte]
#define DEBUG_EE_READ         0  // Debug EEPROM Read Access
#define DEBUG_EE_WRITE        0  // Debug EEPROM Write Access
#define DEBUG_SETUP           1  // Debug Setup 
#define DEBUG_SETUP_MCP       0  // Debug Setup MCP   [386 Byte]
#define DEBUG_IRQ             1  // Debug IRQ
#define DEBUG_HEARTBEAT       1  // Debug Heartbeat
#define DEBUG_OUTPUT          1  // Debug Output
#define DEBUG_STATE_CHANGE    1  // Debug The Change of States
#define DEBUG_SETUP_DELAY     00 // Debug Delay during setup

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

#endif  // _DEBUGOPTIONS_H_