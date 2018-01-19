/**************************************************************************************************
  Filename:       zcl_samplesw.h
  Revised:        $Date: 2015-08-19 17:11:00 -0700 (Wed, 19 Aug 2015) $
  Revision:       $Revision: 44460 $


  Description:    This file contains the Zigbee Cluster Library Home
                  Automation Sample Application.


  Copyright 2006-2013 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

#ifndef ZCL_HOMEGATEWAY_H
#define ZCL_HOMEGATEWAY_H

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
#define HOMEGATEWAY_ENDPOINT           1

#define HOMEGATEWAY_MAX_ATTRIBUTES     15

#define LIGHT_OFF                       0x00
#define LIGHT_ON                        0x01

// Events for the sample app
#define HOMEGATEWAY_IDENTIFY_TIMEOUT_EVT         0x0001
#define HOMEGATEWAY_POLL_CONTROL_TIMEOUT_EVT     0x0002
#define HOMEGATEWAY_EZMODE_TIMEOUT_EVT           0x0004
#define HOMEGATEWAY_EZMODE_NEXTSTATE_EVT         0x0008
#define HOMEGATEWAY_MAIN_SCREEN_EVT              0x0010


// Application Display Modes
#define SW_MAINMODE      0x00
#define SW_HELPMODE      0x01

/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t zclHomeGateway_SimpleDesc;

extern CONST zclAttrRec_t zclHomeGateway_Attrs[];

extern uint8  zclHomeGateway_OnOff;

extern uint16 zclHomeGateway_IdentifyTime;

extern uint8 zclHomeGateway_OnOffSwitchType;

extern uint8 zclHomeGateway_OnOffSwitchActions;

//####################################################################
//####################################################################
extern byte zclHomeGateway_TaskID;
extern devStates_t zclHomeGateway_NwkState;

extern void zclHomeGateway_zclGeneral_SendOnOff_CmdOn(void);
extern void zclHomeGateway_zclGeneral_SendOnOff_CmdOff(void);
extern void zclHomeGateway_zclGeneral_SendOnOff_CmdToggle(void);
extern void zclHomeGateway_zclGeneral_SendLevelControlStepWithOnOff_Up(void);
extern void zclHomeGateway_zclGeneral_SendLevelControlStepWithOnOff_Down(void);
extern void zclHomeGateway_zclGeneral_SendLevelControlMoveWithOnOff_Up(void);
extern void zclHomeGateway_zclGeneral_SendLevelControlMoveWithOnOff_Down(void);
extern void zclHomeGateway_zclGeneral_SendLevelControlStopWithOnOff(void);
extern void zclHomeGateway_zclLighting_ColorControl_Send_StepColorTemperatureCmd_Up(void);
extern void zclHomeGateway_zclLighting_ColorControl_Send_StepColorTemperatureCmd_Down(void);

//####################################################################
//####################################################################

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Initialization for the task
  */
extern void zclHomeGateway_Init( byte task_id );

/*
 *  Event Process for the task
 */
extern UINT16 zclHomeGateway_event_loop( byte task_id, UINT16 events );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_HOMEGATEWAY_H */
