/**************************************************************************************************
  Filename:       zcl_samplelight.h
  Revised:        $Date: 2014-06-19 08:38:22 -0700 (Thu, 19 Jun 2014) $
  Revision:       $Revision: 39101 $

  Description:    This file contains the Zigbee Cluster Library Home
                  Automation Sample Application.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#ifndef ZCL_DIMMERLIGHT_H
#define ZCL_DIMMERLIGHT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

//####################################################################
//####################################################################
#include "add_common.h"
//####################################################################
//####################################################################

/*********************************************************************
 * CONSTANTS
 */
#define DIMMERLIGHT_ENDPOINT            1

#define LIGHT_OFF                       0x00
#define LIGHT_ON                        0x01

// Application Events
#define DIMMERLIGHT_IDENTIFY_TIMEOUT_EVT     0x0001
#define DIMMERLIGHT_POLL_CONTROL_TIMEOUT_EVT 0x0002
#define DIMMERLIGHT_EZMODE_TIMEOUT_EVT       0x0004
#define DIMMERLIGHT_EZMODE_NEXTSTATE_EVT     0x0008
#define DIMMERLIGHT_MAIN_SCREEN_EVT          0x0010
#define DIMMERLIGHT_LEVEL_CTRL_EVT           0x0020
#define DIMMERLIGHT_START_EZMODE_EVT         0x0040  

// Application Display Modes
#define LIGHT_MAINMODE      0x00
#define LIGHT_HELPMODE      0x01

/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t zclDimmerLight_SimpleDesc;

extern CONST zclCommandRec_t zclDimmerLight_Cmds[];

extern CONST uint8 zclCmdsArraySize;

// attribute list
extern CONST zclAttrRec_t zclDimmerLight_Attrs[];
extern CONST uint8 zclDimmerLight_NumAttributes;

// Identify attributes
extern uint16 zclDimmerLight_IdentifyTime;
extern uint8  zclDimmerLight_IdentifyCommissionState;

// OnOff attributes
extern uint8  zclDimmerLight_OnOff;

// Level Control Attributes
#ifdef ZCL_LEVEL_CTRL
extern uint8  zclDimmerLight_LevelCurrentLevel;
extern uint16 zclDimmerLight_LevelRemainingTime;
extern uint16 zclDimmerLight_LevelOnOffTransitionTime;
extern uint8  zclDimmerLight_LevelOnLevel;
extern uint16 zclDimmerLight_LevelOnTransitionTime;
extern uint16 zclDimmerLight_LevelOffTransitionTime;
extern uint8  zclDimmerLight_LevelDefaultMoveRate;
#endif

//####################################################################
//####################################################################
extern byte zclDimmerLight_TaskID;
extern devStates_t zclDimmerLight_NwkState;
//####################################################################
//####################################################################

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Initialization for the task
  */
extern void zclDimmerLight_Init( byte task_id );

/*
 *  Event Process for the task
 */
extern UINT16 zclDimmerLight_event_loop( byte task_id, UINT16 events );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_DIMMERLIGHT_H */
