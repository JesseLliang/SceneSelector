/**************************************************************************************************
  Filename:       zcl_samplesw.c
  Revised:        $Date: 2015-08-19 17:11:00 -0700 (Wed, 19 Aug 2015) $
  Revision:       $Revision: 44460 $

  Description:    Zigbee Cluster Library - sample device application.


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

/*********************************************************************
  This device will be like an On/Off Switch device. This application
  is not intended to be a On/Off Switch device, but will use the device
  description to implement this sample code.

  ----------------------------------------
  Main:
    - SW1: Toggle remote light
    - SW2: Invoke EZMode
    - SW4: Enable/Disable Permit Join
    - SW5: Go to Help screen
  ----------------------------------------
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "MT_SYS.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_dimmerswitch.h"
#include "zcl_ezmode.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
#include "zcl_ota.h"
#include "hal_ota.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zclDimmerSwitch_TaskID;

uint8 zclDimmerSwitchSeqNum;

uint8 zclDimmerSwitch_OnOffSwitchType = ON_OFF_SWITCH_TYPE_TOGGLE;

uint8 zclDimmerSwitch_OnOffSwitchActions = ON_OFF_SWITCH_ACTIONS_2;   // Toggle -> Toggle

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
#ifdef ZCL_ON_OFF
afAddrType_t zclDimmerSwitch_DstAddr;
#endif

#ifdef ZCL_EZMODE
static void zclDimmerSwitch_ProcessZDOMsgs( zdoIncomingMsg_t *pMsg );
static void zclDimmerSwitch_EZModeCB( zlcEZMode_State_t state, zclEZMode_CBData_t *pData );

static const zclEZMode_RegisterData_t zclDimmerSwitch_RegisterEZModeData =
{
  &zclDimmerSwitch_TaskID,
  DIMMERSWITCH_EZMODE_NEXTSTATE_EVT,
  DIMMERSWITCH_EZMODE_TIMEOUT_EVT,
  &zclDimmerSwitchSeqNum,
  zclDimmerSwitch_EZModeCB
};

// NOT ZLC_EZMODE, Use EndDeviceBind
#else

static cId_t bindingOutClusters[] =
{
  ZCL_CLUSTER_ID_GEN_ON_OFF
};
#define ZCLDIMMERSWITCH_BINDINGLIST   (sizeof(bindingOutClusters)/sizeof(bindingOutClusters[0]))
#endif  // ZLC_EZMODE

// Endpoint to allow SYS_APP_MSGs
static endPointDesc_t dimmerSwitch_TestEp =
{
  DIMMERSWITCH_ENDPOINT,                  // endpoint
  &zclDimmerSwitch_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for this test endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

uint8 giSwScreenMode = SW_MAINMODE;   // display the main screen mode first

static uint8 aProcessCmd[] = { 1, 0, 0, 0 }; // used for reset command, { length + cmd0 + cmd1 + data }

uint8 gPermitDuration = 0;    // permit joining default to disabled

devStates_t zclDimmerSwitch_NwkState = DEV_INIT;

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
#define DEVICE_POLL_RATE                 8000   // Poll rate for end device
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclDimmerSwitch_HandleKeys( byte shift, byte keys );
static void zclDimmerSwitch_BasicResetCB( void );
static void zclDimmerSwitch_IdentifyCB( zclIdentify_t *pCmd );
static void zclDimmerSwitch_IdentifyQueryRspCB(  zclIdentifyQueryRsp_t *pRsp );
static void zclDimmerSwitch_ProcessIdentifyTimeChange( void );

// app display functions
void zclDimmerSwitch_LcdDisplayUpdate(void);
void zclDimmerSwitch_LcdDisplayMainMode(void);
void zclDimmerSwitch_LcdDisplayHelpMode(void);

// Functions to process ZCL Foundation incoming Command/Response messages
static void zclDimmerSwitch_ProcessIncomingMsg( zclIncomingMsg_t *msg );
#ifdef ZCL_READ
static uint8 zclDimmerSwitch_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8 zclDimmerSwitch_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif
static uint8 zclDimmerSwitch_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8 zclDimmerSwitch_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclDimmerSwitch_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclDimmerSwitch_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg );
#endif

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
static void zclDimmerSwitch_ProcessOTAMsgs( zclOTA_CallbackMsg_t* pMsg );
#endif

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclDimmerSwitch_CmdCallbacks =
{
  zclDimmerSwitch_BasicResetCB,               // Basic Cluster Reset command
  zclDimmerSwitch_IdentifyCB,                 // Identify command
#ifdef ZCL_EZMODE
  NULL,                                   // Identify EZ-Mode Invoke command
  NULL,                                   // Identify Update Commission State command
#endif
  NULL,                                   // Identify Trigger Effect command
  zclDimmerSwitch_IdentifyQueryRspCB,         // Identify Query Response command
  NULL,                                   // On/Off cluster commands
  NULL,                                   // On/Off cluster enhanced command Off with Effect
  NULL,                                   // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                   // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_LEVEL_CTRL
  NULL,                                   // Level Control Move to Level command
  NULL,                                   // Level Control Move command
  NULL,                                   // Level Control Step command
  NULL,                                   // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  NULL,                                   // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                   // Scene Store Request command
  NULL,                                   // Scene Recall Request command
  NULL,                                   // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                   // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                   // Get Event Log command
  NULL,                                   // Publish Event Log command
#endif
  NULL,                                   // RSSI Location command
  NULL                                    // RSSI Location Response command
};


/*********************************************************************
 * STATUS STRINGS
 */
#ifdef LCD_SUPPORTED
const char sDeviceName[]   = "  Sample Switch";
const char sClearLine[]    = " ";
const char sSwLight[]      = "SW1: ToggleLight";
const char sSwEZMode[]     = "SW2: EZ-Mode";
const char sSwHelp[]       = "SW5: Help";
const char sCmdSent[]      = "  COMMAND SENT";
#endif

/*********************************************************************
 * @fn          zclDimmerSwitch_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void zclDimmerSwitch_Init( byte task_id )
{
  zclDimmerSwitch_TaskID = task_id;

#ifdef ZCL_ON_OFF
  // Set destination address to indirect
  zclDimmerSwitch_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zclDimmerSwitch_DstAddr.endPoint = 0;
  zclDimmerSwitch_DstAddr.addr.shortAddr = 0;
#endif

  // This app is part of the Home Automation Profile
  zclHA_Init( &zclDimmerSwitch_SimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( DIMMERSWITCH_ENDPOINT, &zclDimmerSwitch_CmdCallbacks );

  // Register the application's attribute list
  zcl_registerAttrList( DIMMERSWITCH_ENDPOINT, DIMMERSWITCH_MAX_ATTRIBUTES, zclDimmerSwitch_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( zclDimmerSwitch_TaskID );

#ifdef ZCL_EZMODE
  // Register EZ-Mode
  zcl_RegisterEZMode( &zclDimmerSwitch_RegisterEZModeData );

  // Register with the ZDO to receive Match Descriptor Responses
  ZDO_RegisterForZDOMsg(task_id, Match_Desc_rsp);
#endif

  // Register for all key events - This app will handle all key events
  RegisterForKeys( zclDimmerSwitch_TaskID );

  // Register for a test endpoint
  afRegister( &dimmerSwitch_TestEp );

  ZDO_RegisterForZDOMsg( zclDimmerSwitch_TaskID, End_Device_Bind_rsp );
  ZDO_RegisterForZDOMsg( zclDimmerSwitch_TaskID, Match_Desc_rsp );

#ifdef LCD_SUPPORTED
  HalLcdWriteString ( (char *)sDeviceName, HAL_LCD_LINE_3 );
#endif

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
  // Register for callback events from the ZCL OTA
  zclOTA_Register(zclDimmerSwitch_TaskID);
#endif

//###################################################################
//###################################################################
	addDimmerSwitch_init();
//###################################################################
//###################################################################

}

/*********************************************************************
 * @fn          zclSample_event_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
uint16 zclDimmerSwitch_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclDimmerSwitch_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
#ifdef ZCL_EZMODE
        case ZDO_CB_MSG:
          zclDimmerSwitch_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          break;
#endif
        case ZCL_INCOMING_MSG:
          // Incoming ZCL Foundation command/response messages
          zclDimmerSwitch_ProcessIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          zclDimmerSwitch_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case ZDO_STATE_CHANGE:
          zclDimmerSwitch_NwkState = (devStates_t)(MSGpkt->hdr.status);

//###################################################################
//###################################################################
	addCommon_networkState_led(zclDimmerSwitch_NwkState);
//###################################################################
//###################################################################

          // now on the network
          if ( (zclDimmerSwitch_NwkState == DEV_ZB_COORD) ||
               (zclDimmerSwitch_NwkState == DEV_ROUTER)   ||
               (zclDimmerSwitch_NwkState == DEV_END_DEVICE) )
          {
#ifndef HOLD_AUTO_START
            giSwScreenMode = SW_MAINMODE;
            zclDimmerSwitch_LcdDisplayUpdate();
#endif
#ifdef ZCL_EZMODE
            zcl_EZModeAction( EZMODE_ACTION_NETWORK_STARTED, NULL );
#endif
          }
          break;

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
        case ZCL_OTA_CALLBACK_IND:
          zclDimmerSwitch_ProcessOTAMsgs( (zclOTA_CallbackMsg_t*)MSGpkt  );
          break;
#endif

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & DIMMERSWITCH_IDENTIFY_TIMEOUT_EVT )
  {
    zclDimmerSwitch_IdentifyTime = 10;
    zclDimmerSwitch_ProcessIdentifyTimeChange();

    return ( events ^ DIMMERSWITCH_IDENTIFY_TIMEOUT_EVT );
  }

  if ( events & DIMMERSWITCH_MAIN_SCREEN_EVT )
  {
    giSwScreenMode = SW_MAINMODE;

    zclDimmerSwitch_LcdDisplayUpdate();

#ifdef LCD_SUPPORTED
    HalLcdWriteString( (char *)sClearLine, HAL_LCD_LINE_2 );
#endif
    return ( events ^ DIMMERSWITCH_MAIN_SCREEN_EVT );
  }

#ifdef ZCL_EZMODE
  if ( events & DIMMERSWITCH_EZMODE_NEXTSTATE_EVT )
  {
    zcl_EZModeAction ( EZMODE_ACTION_PROCESS, NULL );   // going on to next state
    return ( events ^ DIMMERSWITCH_EZMODE_NEXTSTATE_EVT );
  }

  if ( events & DIMMERSWITCH_EZMODE_TIMEOUT_EVT )
  {
    zcl_EZModeAction ( EZMODE_ACTION_TIMED_OUT, NULL ); // EZ-Mode timed out
    return ( events ^ DIMMERSWITCH_EZMODE_TIMEOUT_EVT );
  }

#endif // ZLC_EZMODE

//###################################################################
//###################################################################
	return addDimmerSwitch_eventLoop(task_id, events);
//###################################################################
//###################################################################

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      zclDimmerSwitch_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_5
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void zclDimmerSwitch_HandleKeys( byte shift, byte keys )
{
//#####################################################################
//#####################################################################
/*
  // toggle remote light
  if ( keys & HAL_KEY_SW_1 )
  {
    giSwScreenMode = SW_MAINMODE;   // remove help screen if there

    // Using this as the "Light Switch"
#ifdef ZCL_ON_OFF
    zclGeneral_SendOnOff_CmdToggle( DIMMERSWITCH_ENDPOINT, &zclDimmerSwitch_DstAddr, FALSE, 0 );
#endif
#ifdef LCD_SUPPORTED
    HalLcdWriteString( (char *)sCmdSent, HAL_LCD_LINE_2 );

    // clear message on screen after 3 seconds
    osal_start_timerEx( zclDimmerSwitch_TaskID, DIMMERSWITCH_MAIN_SCREEN_EVT, 3000 );
#endif
  }

  // invoke EZ-Mode
  if ( keys & HAL_KEY_SW_2 )
  {
    giSwScreenMode = SW_MAINMODE;   // remove help screen if there

#ifdef ZCL_EZMODE
    {
      zclEZMode_InvokeData_t ezModeData;
      static uint16 clusterIDs[] = { ZCL_CLUSTER_ID_GEN_ON_OFF };   // only bind on the on/off cluster

      // Invoke EZ-Mode
      ezModeData.endpoint = DIMMERSWITCH_ENDPOINT; // endpoint on which to invoke EZ-Mode
      if ( (zclDimmerSwitch_NwkState == DEV_ZB_COORD) ||
               (zclDimmerSwitch_NwkState == DEV_ROUTER)   ||
               (zclDimmerSwitch_NwkState == DEV_END_DEVICE) )
      {
        ezModeData.onNetwork = TRUE;      // node is already on the network
      }
      else
      {
        ezModeData.onNetwork = FALSE;     // node is not yet on the network
      }
      ezModeData.initiator = TRUE;        // OnOffSwitch is an initiator
      ezModeData.numActiveOutClusters = 1;   // active output cluster
      ezModeData.pActiveOutClusterIDs = clusterIDs;
      ezModeData.numActiveInClusters = 0;  // no active input clusters
      ezModeData.pActiveInClusterIDs = NULL;
      zcl_InvokeEZMode( &ezModeData );

 #ifdef LCD_SUPPORTED
      HalLcdWriteString( "EZMode", HAL_LCD_LINE_2 );
 #endif
    }

#else // NOT ZCL_EZMODE
    // bind to remote light
    zAddrType_t dstAddr;
    HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );

    // Initiate an End Device Bind Request, this bind request will
    // only use a cluster list that is important to binding.
    dstAddr.addrMode = afAddr16Bit;
    dstAddr.addr.shortAddr = 0;   // Coordinator makes the match
    ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(),
                           DIMMERSWITCH_ENDPOINT,
                           ZCL_HA_PROFILE_ID,
                           0, NULL,   // No incoming clusters to bind
                           ZCLDIMMERSWITCH_BINDINGLIST, bindingOutClusters,
                           TRUE );
#endif // ZCL_EZMODE
  }

  // toggle permit join
  if ( keys & HAL_KEY_SW_4 )
  {
    giSwScreenMode = SW_MAINMODE;   // remove help screen if there

    if ( ( zclDimmerSwitch_NwkState == DEV_ZB_COORD ) ||
         ( zclDimmerSwitch_NwkState == DEV_ROUTER ) )
    {
      zAddrType_t tmpAddr;

      tmpAddr.addrMode = Addr16Bit;
      tmpAddr.addr.shortAddr = NLME_GetShortAddr();

      // toggle permit join
      gPermitDuration = gPermitDuration ? 0 : 0xff;

      // Trust Center significance is always true
      ZDP_MgmtPermitJoinReq( &tmpAddr, gPermitDuration, TRUE, FALSE );
    }
  }

  if ( shift && ( keys & HAL_KEY_SW_5 ) )
  {
    zclDimmerSwitch_BasicResetCB();
  }
  else if ( keys & HAL_KEY_SW_5 )
  {
    giSwScreenMode = giSwScreenMode ? SW_MAINMODE : SW_HELPMODE;
#ifdef LCD_SUPPORTED
    HalLcdWriteString( (char *)sClearLine, HAL_LCD_LINE_2 );
#endif
  }

  // update the display
  zclDimmerSwitch_LcdDisplayUpdate();
*/
//#####################################################################
//#####################################################################
}

/*********************************************************************
 * @fn      zclDimmerSwitch_LcdDisplayUpdate
 *
 * @brief   Called to update the LCD display.
 *
 * @param   none
 *
 * @return  none
 */
void zclDimmerSwitch_LcdDisplayUpdate(void)
{
//#####################################################################
//#####################################################################
/*
  if ( giSwScreenMode == SW_HELPMODE )
  {
    zclDimmerSwitch_LcdDisplayHelpMode();
  }
  else
  {
    zclDimmerSwitch_LcdDisplayMainMode();
  }
*/
//#####################################################################
//#####################################################################
}

/*********************************************************************
 * @fn      zclDimmerSwitch_LcdDisplayMainMode
 *
 * @brief   Called to display the main screen on the LCD.
 *
 * @param   none
 *
 * @return  none
 */
void zclDimmerSwitch_LcdDisplayMainMode(void)
{
//#####################################################################
//#####################################################################
/*
  if ( zclDimmerSwitch_NwkState == DEV_ZB_COORD )
  {
    zclHA_LcdStatusLine1(0);
  }
  else if ( zclDimmerSwitch_NwkState == DEV_ROUTER )
  {
    zclHA_LcdStatusLine1(1);
  }
  else if ( zclDimmerSwitch_NwkState == DEV_END_DEVICE )
  {
    zclHA_LcdStatusLine1(2);
  }

#ifdef LCD_SUPPORTED
  if ( ( zclDimmerSwitch_NwkState == DEV_ZB_COORD ) ||
       ( zclDimmerSwitch_NwkState == DEV_ROUTER ) )
  {
    // display help key with permit join status
    if ( gPermitDuration )
    {
      HalLcdWriteString("SW5: Help      *", HAL_LCD_LINE_3);
    }
    else
    {
      HalLcdWriteString("SW5: Help       ", HAL_LCD_LINE_3);
    }
  }
  else
  {
    // display help key
    HalLcdWriteString((char *)sSwHelp, HAL_LCD_LINE_3);
  }
#endif
*/
//#####################################################################
//#####################################################################
}

/*********************************************************************
 * @fn      zclDimmerSwitch_LcdDisplayHelpMode
 *
 * @brief   Called to display the SW options on the LCD.
 *
 * @param   none
 *
 * @return  none
 */
void zclDimmerSwitch_LcdDisplayHelpMode(void)
{
//#####################################################################
//#####################################################################
/*
#ifdef LCD_SUPPORTED
  HalLcdWriteString( (char *)sSwLight, HAL_LCD_LINE_1 );
  HalLcdWriteString( (char *)sSwEZMode, HAL_LCD_LINE_2 );
  HalLcdWriteString( (char *)sSwHelp, HAL_LCD_LINE_3 );
#endif
*/
//#####################################################################
//#####################################################################
}

/*********************************************************************
 * @fn      zclDimmerSwitch_ProcessIdentifyTimeChange
 *
 * @brief   Called to process any change to the IdentifyTime attribute.
 *
 * @param   none
 *
 * @return  none
 */
static void zclDimmerSwitch_ProcessIdentifyTimeChange( void )
{
  if ( zclDimmerSwitch_IdentifyTime > 0 )
  {
    osal_start_timerEx( zclDimmerSwitch_TaskID, DIMMERSWITCH_IDENTIFY_TIMEOUT_EVT, 1000 );
    HalLedBlink ( HAL_LED_4, 0xFF, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME );
  }
  else
  {
    if ( zclDimmerSwitch_OnOff )
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_ON );
    else
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
    osal_stop_timerEx( zclDimmerSwitch_TaskID, DIMMERSWITCH_IDENTIFY_TIMEOUT_EVT );
  }
}

/*********************************************************************
 * @fn      zclDimmerSwitch_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to  default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclDimmerSwitch_BasicResetCB( void )
{
  // Put device back to factory default settings
  zgWriteStartupOptions( ZG_STARTUP_SET, 3 );   // bit set both default configuration and default network

  // restart device
  MT_SysCommandProcessing( aProcessCmd );
}

/*********************************************************************
 * @fn      zclDimmerSwitch_IdentifyCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Command for this application.
 *
 * @param   srcAddr - source address and endpoint of the response message
 * @param   identifyTime - the number of seconds to identify yourself
 *
 * @return  none
 */
static void zclDimmerSwitch_IdentifyCB( zclIdentify_t *pCmd )
{
  zclDimmerSwitch_IdentifyTime = pCmd->identifyTime;
  zclDimmerSwitch_ProcessIdentifyTimeChange();
}

/*********************************************************************
 * @fn      zclDimmerSwitch_IdentifyQueryRspCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Query Response Command for this application.
 *
 * @param   srcAddr - source address
 * @param   timeout - number of seconds to identify yourself (valid for query response)
 *
 * @return  none
 */
static void zclDimmerSwitch_IdentifyQueryRspCB(  zclIdentifyQueryRsp_t *pRsp )
{
  (void)pRsp;
#ifdef ZCL_EZMODE
  {
    zclEZMode_ActionData_t data;
    data.pIdentifyQueryRsp = pRsp;
    zcl_EZModeAction ( EZMODE_ACTION_IDENTIFY_QUERY_RSP, &data );
  }
#endif
}

/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      zclDimmerSwitch_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static void zclDimmerSwitch_ProcessIncomingMsg( zclIncomingMsg_t *pInMsg )
{
  switch ( pInMsg->zclHdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclDimmerSwitch_ProcessInReadRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclDimmerSwitch_ProcessInWriteRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_REPORT
    // See ZCL Test Applicaiton (zcl_testapp.c) for sample code on Attribute Reporting
    case ZCL_CMD_CONFIG_REPORT:
      //zclDimmerSwitch_ProcessInConfigReportCmd( pInMsg );
      break;

    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclDimmerSwitch_ProcessInConfigReportRspCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      //zclDimmerSwitch_ProcessInReadReportCfgCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclDimmerSwitch_ProcessInReadReportCfgRspCmd( pInMsg );
      break;

    case ZCL_CMD_REPORT:
      //zclDimmerSwitch_ProcessInReportCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zclDimmerSwitch_ProcessInDefaultRspCmd( pInMsg );
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      zclDimmerSwitch_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      zclDimmerSwitch_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclDimmerSwitch_ProcessInDiscAttrsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      zclDimmerSwitch_ProcessInDiscAttrsExtRspCmd( pInMsg );
      break;
#endif
    default:
      break;
  }

  if ( pInMsg->attrCmd )
    osal_mem_free( pInMsg->attrCmd );
}

#ifdef ZCL_READ
/*********************************************************************
 * @fn      zclDimmerSwitch_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclDimmerSwitch_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8 i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < readRspCmd->numAttr; i++)
  {
    // Notify the originator of the results of the original read attributes
    // attempt and, for each successfull request, the value of the requested
    // attribute
  }

  return TRUE;
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclDimmerSwitch_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclDimmerSwitch_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8 i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < writeRspCmd->numAttr; i++)
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return TRUE;
}
#endif // ZCL_WRITE

/*********************************************************************
 * @fn      zclDimmerSwitch_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclDimmerSwitch_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;
  // Device is notified of the Default Response command.
  (void)pInMsg;
  return TRUE;
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclDimmerSwitch_ProcessInDiscCmdsRspCmd
 *
 * @brief   Process the Discover Commands Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclDimmerSwitch_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverCmdsCmdRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverCmdsCmdRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numCmd; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return TRUE;
}

/*********************************************************************
 * @fn      zclDimmerSwitch_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclDimmerSwitch_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsRspCmd_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return TRUE;
}

/*********************************************************************
 * @fn      zclDimmerSwitch_ProcessInDiscAttrsExtRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Extended Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclDimmerSwitch_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsExtRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsExtRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return TRUE;
}
#endif // ZCL_DISCOVER

#if ZCL_EZMODE
/*********************************************************************
 * @fn      zclDimmerSwitch_ProcessZDOMsgs
 *
 * @brief   Called when this node receives a ZDO/ZDP response.
 *
 * @param   none
 *
 * @return  status
 */
static void zclDimmerSwitch_ProcessZDOMsgs( zdoIncomingMsg_t *pMsg )
{

  // Let EZ-Mode know of the Match Descriptor Reponse (same as ActiveEP Response)
  if ( pMsg->clusterID == Match_Desc_rsp )
  {
    zclEZMode_ActionData_t data;
    ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( pMsg );
    data.pMatchDescRsp = pRsp;
    zcl_EZModeAction( EZMODE_ACTION_MATCH_DESC_RSP, &data );
    osal_mem_free(pRsp);
  }
}

/*********************************************************************
 * @fn      zclDimmerSwitch_EZModeCB
 *
 * @brief   The Application is informed of events. This can be used to show on the UI what is
*           going on during EZ-Mode steering/finding/binding.
 *
 * @param   state - an
 *
 * @return  none
 */
static void zclDimmerSwitch_EZModeCB( zlcEZMode_State_t state, zclEZMode_CBData_t *pData )
{
#ifdef LCD_SUPPORTED
  char szLine[20];
  char *pStr;
  uint8 err;
#endif

  // time to go into identify mode
  if ( state == EZMODE_STATE_IDENTIFYING )
  {
    zclDimmerSwitch_IdentifyTime = (EZMODE_TIME / 1000);  // convert to seconds
    zclDimmerSwitch_ProcessIdentifyTimeChange();
  }

  // autoclosing, show what happened (success, cancelled, etc...)
  if( state == EZMODE_STATE_AUTOCLOSE )
  {
#ifdef LCD_SUPPORTED
    pStr = NULL;
    err = pData->sAutoClose.err;
    if ( err == EZMODE_ERR_SUCCESS )
    {
      pStr = "EZMode: Success";
    }
    else if ( err == EZMODE_ERR_NOMATCH )
    {
      pStr = "EZMode: NoMatch"; // not a match made in heaven
    }
    if ( pStr )
    {
      if ( giSwScreenMode == SW_MAINMODE )
        HalLcdWriteString ( pStr, HAL_LCD_LINE_2 );
    }
#endif
  }

  // finished, either show DstAddr/EP, or nothing (depending on success or not)
  if( state == EZMODE_STATE_FINISH )
  {
    // turn off identify mode
    zclDimmerSwitch_IdentifyTime = 0;
    zclDimmerSwitch_ProcessIdentifyTimeChange();

#ifdef LCD_SUPPORTED
    // if successful, inform user which nwkaddr/ep we bound to
    pStr = NULL;
    err = pData->sFinish.err;
    if( err == EZMODE_ERR_SUCCESS )
    {
      // "EZDst:1234 EP:34"
      osal_memcpy(szLine, "EZDst:", 6);
      zclHA_uint16toa( pData->sFinish.nwkaddr, &szLine[6]);
      osal_memcpy(&szLine[10], " EP:", 4);
      _ltoa( pData->sFinish.ep, (void *)(&szLine[14]), 16 );  // _ltoa NULL terminates
      pStr = szLine;
    }
    else if ( err == EZMODE_ERR_BAD_PARAMETER )
    {
      pStr = "EZMode: BadParm";
    }
    else if ( err == EZMODE_ERR_CANCELLED )
    {
      pStr = "EZMode: Cancel";
    }
    else
    {
      pStr = "EZMode: TimeOut";
    }
    if ( pStr )
    {
      if ( giSwScreenMode == SW_MAINMODE )
        HalLcdWriteString ( pStr, HAL_LCD_LINE_2 );
    }
#endif  // LCD_SUPPORTED
    // show main UI screen 3 seconds after binding
    osal_start_timerEx( zclDimmerSwitch_TaskID, DIMMERSWITCH_MAIN_SCREEN_EVT, 3000 );
  }

}

#endif // ZCL_EZMODE

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
/*********************************************************************
 * @fn      zclDimmerSwitch_ProcessOTAMsgs
 *
 * @brief   Called to process callbacks from the ZCL OTA.
 *
 * @param   none
 *
 * @return  none
 */
static void zclDimmerSwitch_ProcessOTAMsgs( zclOTA_CallbackMsg_t* pMsg )
{
  uint8 RxOnIdle;

  switch(pMsg->ota_event)
  {
  case ZCL_OTA_START_CALLBACK:
    if (pMsg->hdr.status == ZSuccess)
    {
      // Speed up the poll rate
      RxOnIdle = TRUE;
      ZMacSetReq( ZMacRxOnIdle, &RxOnIdle );
      NLME_SetPollRate( 2000 );
    }
    break;

  case ZCL_OTA_DL_COMPLETE_CALLBACK:
    if (pMsg->hdr.status == ZSuccess)
    {
      // Reset the CRC Shadow and reboot.  The bootloader will see the
      // CRC shadow has been cleared and switch to the new image
      HalOTAInvRC();
      SystemReset();
    }
    else
    {
      // slow the poll rate back down.
      RxOnIdle = FALSE;
      ZMacSetReq( ZMacRxOnIdle, &RxOnIdle );
      NLME_SetPollRate(DEVICE_POLL_RATE);
    }
    break;

  default:
    break;
  }
}
#endif // defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)

/****************************************************************************
****************************************************************************/

