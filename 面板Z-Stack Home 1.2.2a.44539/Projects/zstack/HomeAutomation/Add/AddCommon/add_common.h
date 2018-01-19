#ifndef __ADD_COMMON_H__
#define __ADD_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <ioCC2530.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"

#include "onboard.h"

#include "osal_nv.h"

#include "ZDObject.h"

#include "OSAL_PwrMgr.h"

#include "Hal_drivers.h"

#include "MT_APP.h"
#include "MT_SYS.h"

#include "zcl_general.h"
#include "zcl_lighting.h"

#if defined(HOME_GATEWAY)
#include "add_homegateway.h"
#include "zcl_homegateway.h"

#elif defined(DIMMER_SWITCH) || defined(DIMMER_SWITCH_TOUCHKEY)
#include "add_dimmerswitch.h"
#include "zcl_dimmerswitch.h"

#elif defined(SCENE_SELECTOR) || defined(SCENE_SELECTOR_TOUCHKEY)
#include "add_sceneselector.h"
#include "zcl_sceneselector.h"

#elif defined(DIMMER_LIGHT)
#include "add_dimmerlight.h"
#include "zcl_dimmerlight.h"

#else
#endif

/*********************************************************************
 * MACROS
 */
#define AC_DEVICE_LED_NETWORK_STATE P0_6 // p0_6 indicates the device network state

#define AC_DEVICE_SETUP_KEY P1_2 // setup key

/*********************************************************************
 * CONSTANTS
 */
//--------------------------------------------------------------------
//event
#define AC_EVENT_POLLING 0x0100
#define AC_EVENT_HANDLE_ONE_TIME 0x0200
#define AC_EVENT_DEVICE_LED_NETWORK_STATE 0x0400
#define AC_EVENT_DEVICE_KEY_TIMING 0x0800
#define AC_EVENT_DOUBLE_CLICK_CHECK 0x2000

//--------------------------------------------------------------------
// INTERRUPT TRIGGER EDGE
#define AC_INTERRUPT_TRIGGER_EDGE_FALL 0x00
#define AC_INTERRUPT_TRIGGER_EDGE_RISE 0x01

//--------------------------------------------------------------------
// setup key status
#define AC_KEY_NORMAL 0x00
#define AC_KEY_DOWN 0x01
#define AC_KEY_UP_BELOW_600MS 0x02
#define AC_KEY_UP_600MS_TO_1S 0x03
#define AC_KEY_UP_1S_TO_5S 0x04
#define AC_KEY_UP_5S_TO_10S 0x05
#define AC_KEY_UP_ABOVE_10S 0x06

//--------------------------------------------------------------------
// delay
#define AC_DELAY_POLLING 5000 // 5000ms
#define AC_DELAY_POWER_ON 1 // 1 * 5000ms
#define AC_DELAY_JOIN_NETWORK 24 // 24 * 5000ms
#define AC_DELAY_DEEP_SLEEP_AFTER_JOIN_NETWORK 12 // 12 * 5000ms
#define AC_DELAY_LED_NETWORK_STATE 1500 // 1500ms
#define AC_DELAY_EXIT_NETWORK_CLEAR_FLAG_INTERVAL 5000 //5000ms
#define AC_DELAY_DOUBLE_CLICK 600 //600ms

//--------------------------------------------------------------------
// network
#define AC_NETWORK_JOIN_PERMIT_DURATION 60 // second(s)

/*********************************************************************
 * VARIABLES
 */
extern byte *acPointer_TaskID;
extern devStates_t *acPointer_NwkState;

extern volatile bool acSleepFlag;

extern uint8 acDelayPowerOn;
extern uint8 acDelayJoinNetwork;

extern const uint16 acDeviceKeyTimingUnit;
extern uint8 acDeviceKeyTimingCounter; 
extern byte acDeviceSetupKeyStatus;

extern volatile uint8 acDoubleClickCounter_whenKeyUp;
extern volatile uint8 acDoubleClickCounter_whenKeyDown;
extern volatile bool acDoubleClickCheckFlag;

extern volatile byte acInterruptTriggerEdgePort0;
extern volatile byte acInterruptTriggerEdgePort1;
extern volatile byte acInterruptTriggerEdgePort2;

/*********************************************************************
 * FUNCTIONS
 */
extern void addCommon_init(void);
extern uint16 addCommon_eventLoop( uint8 task_id, uint16 events );
extern void addCommon_polling(void);
extern void addCommon_handleOneTime(void);
extern void addCommon_isrPort0(void);
extern void addCommon_isrPort1(void);
extern void addCommon_isrPort2(void);

extern void addCommon_delayMS(uint16 delayMS);
extern void addCommon_ledFlash(uint8 flashCount, uint16 flashDelay);
extern bool addCommon_atNormalWorkState(bool judgeNetworkState);
extern void addCommon_networkState_led(devStates_t nwkState);
extern uint8 addCommon_handleKeyDown(void);
extern uint8 addCommon_handleKeyUp(void);
extern void addCommon_sleep(void);
extern void addCommon_wakeup(void);
extern void addCommon_factoryReset(void);

#ifdef __cplusplus
}
#endif
 
#endif /* __ADD_COMMON_H__ */
