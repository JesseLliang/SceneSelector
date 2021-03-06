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

#ifndef ZCL_SCENESELECTOR_H
#define ZCL_SCENESELECTOR_H

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
//#define SCENESELECTOR_ENDPOINT           8
//####################################################################
//####################################################################
#define SCENESELECTOR_ENDPOINT1           1
#define SCENESELECTOR_ENDPOINT2           2
#define SCENESELECTOR_ENDPOINT3           3
#define SCENESELECTOR_ENDPOINT4           4
//####################################################################
//####################################################################

#define SCENESELECTOR_MAX_ATTRIBUTES     15

#define LIGHT_OFF                       0x00
#define LIGHT_ON                        0x01

// Events for the sample app
#define SCENESELECTOR_IDENTIFY_TIMEOUT_EVT         0x0001
#define SCENESELECTOR_POLL_CONTROL_TIMEOUT_EVT     0x0002
#define SCENESELECTOR_EZMODE_TIMEOUT_EVT           0x0004
#define SCENESELECTOR_EZMODE_NEXTSTATE_EVT         0x0008
#define SCENESELECTOR_MAIN_SCREEN_EVT              0x0010

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
//extern SimpleDescriptionFormat_t zclSceneSelector_SimpleDesc;
//###########################################################################
//###########################################################################
extern SimpleDescriptionFormat_t zclSceneSelector_SimpleDesc1;
extern SimpleDescriptionFormat_t zclSceneSelector_SimpleDesc2;
extern SimpleDescriptionFormat_t zclSceneSelector_SimpleDesc3;
extern SimpleDescriptionFormat_t zclSceneSelector_SimpleDesc4;
//###########################################################################
//###########################################################################

extern CONST zclAttrRec_t zclSceneSelector_Attrs[];

extern uint8  zclSceneSelector_OnOff;

extern uint16 zclSceneSelector_IdentifyTime;

extern uint8 zclSceneSelector_OnOffSwitchType;

extern uint8 zclSceneSelector_OnOffSwitchActions;

//####################################################################
//####################################################################
extern afAddrType_t zclSceneSelector_DstAddr;

extern byte zclSceneSelector_TaskID;
extern devStates_t zclSceneSelector_NwkState;
//####################################################################
//####################################################################

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Initialization for the task
  */
extern void zclSceneSelector_Init( byte task_id );

/*
 *  Event Process for the task
 */
extern UINT16 zclSceneSelector_event_loop( byte task_id, UINT16 events );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SAMPLEAPP_H */
