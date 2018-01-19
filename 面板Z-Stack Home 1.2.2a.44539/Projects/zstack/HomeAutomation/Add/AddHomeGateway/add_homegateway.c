/*********************************************************************
 * INCLUDES
 */
#include "add_homegateway.h"

/*********************************************************************
 * LOCAL VARIABLES
 */
volatile bool handleOneTimeFinished = TRUE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void addHomeGateway_init(void);
static void addHomeGateway_initHal(void);
static void addHomeGateway_initAl(void);
static void addHomeGateway_initNvItemID(void);
static void addHomeGateway_initNvData(void);
uint16 addHomeGateway_eventLoop(uint8 task_id, uint16 events);
static void addHomeGateway_polling(void);
static void addHomeGateway_handleOneTime(void);
void addHomeGateway_isrPort0(void);
void addHomeGateway_isrPort1(void);
void addHomeGateway_isrPort2(void);

/*********************************************************************
 * @fn      addHomeGateway_init
 *
 * @brief   Dimmer switch initial.
 *
 * @param   none
 *
 * @return  none
 */
void addHomeGateway_init(void)
{
	addCommon_init();

	addHomeGateway_initHal();
	addCommon_delayMS(500);
	addHomeGateway_initAl();

	acPointer_NwkState = &zclHomeGateway_NwkState;
	acPointer_TaskID = &zclHomeGateway_TaskID;
	osal_start_timerEx(zclHomeGateway_TaskID, AC_EVENT_POLLING, AC_DELAY_POLLING);

#if defined(POWER_SAVING)
	//(void)osal_pwrmgr_task_state(zclHomeGateway_TaskID, PWRMGR_HOLD);
	NLME_SetPollRate(1000);
#else
#endif
}

/*********************************************************************
 * @fn      addHomeGateway_initHal
 *
 * @brief   Home gateway initial hardware abstract layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addHomeGateway_initHal(void)
{		
	// no operation
}

/*********************************************************************
 * @fn      addHomeGateway_initAl
 *
 * @brief   Home gateway initial application layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addHomeGateway_initAl(void)
{	
	addHomeGateway_initNvItemID();
	addHomeGateway_initNvData();
}

/*********************************************************************
 * @fn      addHomeGateway_initNvItemID
 *
 * @brief   Home gateway initial non-volatile flash item ID.
 *
 * @param   none
 *
 * @return  none
 */
static void addHomeGateway_initNvItemID(void)
{
	// no operation
}

/*********************************************************************
 * @fn      addHomeGateway_initNvData
 *
 * @brief   Home gateway initial non-volatile flash data.
 *
 * @param   none
 *
 * @return  none
 */
 static void addHomeGateway_initNvData(void)
{
	// no opeartion
}

/*********************************************************************
 * @fn      addHomeGateway_eventLoop
 *
 * @brief   Home gateway Event Loop Processor for zclGeneral.
 *
 * @param   uint8 task_id, uint16 events
 *
 * @return  uint16
 */
uint16 addHomeGateway_eventLoop(uint8 task_id, uint16 events)
{		
	if (events & AC_EVENT_POLLING)
	{
		addHomeGateway_polling();
		//osal_start_timerEx(zclHomeGateway_TaskID, AC_EVENT_POLLING, AC_DELAY_POLLING);
		if (acDelayJoinNetwork != 0)
		{
			osal_start_timerEx(zclHomeGateway_TaskID, AC_EVENT_POLLING, AC_DELAY_POLLING);
		} else
		{		
#if defined(POWER_SAVING)
			//(void)osal_pwrmgr_task_state(zclHomeGateway_TaskID, PWRMGR_CONSERVE);
			NLME_SetPollRate(10000);
#else
#endif
		}
		return ( events ^ AC_EVENT_POLLING );
	}
	
	if (events & AC_EVENT_HANDLE_ONE_TIME)
	{			
		addHomeGateway_handleOneTime();
		handleOneTimeFinished = TRUE;
		return ( events ^ AC_EVENT_HANDLE_ONE_TIME);
	}
	
	return addCommon_eventLoop(task_id, events);
}

/*********************************************************************
 * @fn      addHomeGateway_polling
 *
 * @brief   Home gateway polling.
 *
 * @param   none
 *
 * @return  none
 */
static void addHomeGateway_polling(void)
{
	addCommon_polling();
}

/*********************************************************************
 * @fn      addHomeGateway_handleOneTime
 *
 * @brief   Home gateway handle one time.
 *
 * @param   none
 *
 * @return  none
 */
void addHomeGateway_handleOneTime(void)
{
//	uint8 bTemp = 0;

handleOneTimeFinished = FALSE;

	addCommon_handleOneTime();
	
	if (acDeviceSetupKeyStatus == AC_KEY_UP_1S_TO_5S) // setup key
	{
		acDeviceSetupKeyStatus = AC_KEY_NORMAL;
		//zclHomeGateway_invokeEZMODE();
	} else if (acDeviceSetupKeyStatus == AC_KEY_UP_5S_TO_10S)
	{
		acDeviceSetupKeyStatus = AC_KEY_NORMAL;		
		if (( zclHomeGateway_NwkState == DEV_ZB_COORD ) ||
			( zclHomeGateway_NwkState == DEV_ROUTER ))
		{
			NLME_PermitJoiningRequest(AC_NETWORK_JOIN_PERMIT_DURATION);
		}
	} else if (acDeviceSetupKeyStatus == AC_KEY_UP_ABOVE_10S)
	{
		acDeviceSetupKeyStatus = AC_KEY_NORMAL;
		addCommon_factoryReset();
	}
}

/*********************************************************************
 * @fn      addHomeGateway_isrPort0
 *
 * @brief   Port0 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addHomeGateway_isrPort0(void)
{
	// no operation
}

/*********************************************************************
 * @fn      addHomeGateway_isrPort1
 *
 * @brief   Port1 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addHomeGateway_isrPort1(void)
{
	// no operation
}

/*********************************************************************
 * @fn      addHomeGateway_isrPort2
 *
 * @brief   Port2 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addHomeGateway_isrPort2(void)
{
	// no operation
}

