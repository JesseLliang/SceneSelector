#ifndef __ADD_DIMMERLIGHT_H__
#define __ADD_DIMMERLIGHT_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "add_common.h"

/*********************************************************************
 * MACROS
 */
#define ADD_DIMMERLIGHT_CW  P1_6 // cold white
#define ADD_DIMMERLIGHT_WW P2_0 // warm white

/*********************************************************************
 * CONSTANTS
 */
// OSAL NV item IDs
#define A_DIMMERLIGHT_NV_ITEM_ID_CW 0x0501 // 1byte
#define A_DIMMERLIGHT_NV_ITEM_ID_WW 0x0502 // 1byte
#define A_DIMMERLIGHT_NV_ITEM_ID_LEAVE_NETWORK_COUNT 0x0503 // 1byte

// event
#define AC_EVENT_DEVICE_LEAVE_NETWORK 0x1000

#define A_DIMMERLIGHT_LEVEL_MIN 0
#define A_DIMMERLIGHT_LEVEL_10PERCENT 26
#define A_DIMMERLIGHT_LEVEL_MAX 255

/*********************************************************************
 * VARIABLES
 */
extern byte aDimmerLightCW;
extern byte aDimmerLightWW;
extern byte aDimmerLightLeaveNetworkCount;

/*********************************************************************
 * FUNCTIONS
 */
extern void addDimmerLight_init(void);
extern uint16 addDimmerLight_eventLoop(uint8 task_id, uint16 events);
extern void addDimmerLight_isrPort0(void);
extern void addDimmerLight_isrPort1(void);
extern void addDimmerLight_isrPort2(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADD_DIMMERLIGHT_H__ */

