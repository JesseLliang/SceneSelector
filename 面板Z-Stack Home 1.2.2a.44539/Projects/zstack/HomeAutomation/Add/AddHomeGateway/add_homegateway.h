#ifndef __ADD_HOMEGATEWAY_H__
#define __ADD_HOMEGATEWAY_H__

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

/*********************************************************************
 * CONSTANTS
 */
// OSAL NV item IDs

// event

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */
extern void addHomeGateway_init(void);
extern uint16 addHomeGateway_eventLoop(uint8 task_id, uint16 events);
extern void addHomeGateway_isrPort0(void);
extern void addHomeGateway_isrPort1(void);
extern void addHomeGateway_isrPort2(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADD_HOMEGATEWAY_H__ */

