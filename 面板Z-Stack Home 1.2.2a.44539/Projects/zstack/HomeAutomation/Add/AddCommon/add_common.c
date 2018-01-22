/*********************************************************************
 * INCLUDES
 */
#include "add_common.h"

/*********************************************************************
 * LOCAL VARIABLES
 */
byte *acPointer_TaskID;
devStates_t *acPointer_NwkState;
bool acNwkState_stopFlash = TRUE;

volatile bool acSleepFlag = FALSE;

uint8 acDelayPowerOn = AC_DELAY_POWER_ON;
uint8 acDelayJoinNetwork = AC_DELAY_JOIN_NETWORK;

const uint16 acDeviceKeyTimingUnit = 100; // 100ms
uint8 acDeviceKeyTimingCounter = 0; 
byte acDeviceSetupKeyStatus = AC_KEY_NORMAL;

volatile uint8 acDoubleClickCounter_whenKeyUp = 0;
volatile uint8 acDoubleClickCounter_whenKeyDown = 0;
volatile bool acDoubleClickCheckFlag = FALSE;

volatile byte acInterruptTriggerEdgePort0 = AC_INTERRUPT_TRIGGER_EDGE_FALL;
volatile byte acInterruptTriggerEdgePort1 = AC_INTERRUPT_TRIGGER_EDGE_FALL;
volatile byte acInterruptTriggerEdgePort2 = AC_INTERRUPT_TRIGGER_EDGE_FALL;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void addCommon_init(void);
static void addCommon_initHal(void);
static void addCommon_initAl(void);
static void addCommon_initNvItemID(void);
static void addCommon_initNvData(void);
uint16 addCommon_eventLoop( uint8 task_id, uint16 events );
void addCommon_polling(void);
void addCommon_handleOneTime(void);
void addCommon_isrPort0(void);
void addCommon_isrPort1(void);
void addCommon_isrPort2(void);

void addCommon_delayMS(uint16 delayMS);
void addCommon_ledFlash(uint8 flashCount, uint16 flashDelay);
bool addCommon_atNormalWorkState(bool judgeNetworkState);
void addCommon_networkState_led(devStates_t nwkState);
static void addCommon_handleKeyEvent(void);
uint8 addCommon_handleKeyDown(void);
uint8 addCommon_handleKeyUp(void);
static void NLME_SetRejoinPollRate(uint32 newRate);
void addCommon_sleep(void);
void addCommon_wakeup(void);
void addCommon_factoryReset(void);

/*********************************************************************
 * @fn      addCommon_init
 *
 * @brief   Initial.
 *
 * @param   none
 *
 * @return  none
 */
void addCommon_init(void)
{
	addCommon_initHal();	
	addCommon_delayMS(500);
	addCommon_initAl();
}

/*********************************************************************
 * @fn      addCommon_initHal
 *
 * @brief   Common initial hardware abstract layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addCommon_initHal(void)
{
	APCFG = 0x00;
	P0SEL = 0x00;
	P0INP = 0x00;
	P0DIR = 0x00;
	P1SEL = 0x00;
	P1INP = 0x00;
	P1DIR = 0x00;
	P2INP = 0x00;
	P2DIR &= ~0x1f;
	
	APCFG &= ~0x40; // P0_6 indicates the device network state
	P0SEL &= ~0x40; 
	P0DIR |= 0x40;
	P0INP |= 0x40;
	
	AC_DEVICE_LED_NETWORK_STATE = 0;
	
	P1SEL &= ~0x04; // Setup key on port P1_2
	P1DIR &= ~0x04;
	P2INP &= ~0x40; // port1 all pull up
	P1INP &= ~0x04;

	EA = 0;
	
	MicroWait(50000);
	MicroWait(50000);

	P1IEN |= 0x04; // P1_2 interrupt enable
	PICTL |= 0x02; // fall edge
	P1IFG &= ~0x04; // clear interrupt flag
	IEN2 |= 0x10; // port1 interrupt enable
	
	IP0 |= (1 << 4); // set port1 interrupt highest priority
	IP1 |= (1 << 4);
	
	EA = 1;
}

/*********************************************************************
 * @fn      addCommon_initAl
 *
 * @brief   Common init.
 *
 * @param   none
 *
 * @return  none
 */
static void addCommon_initAl(void)
{
	addCommon_initNvItemID();
	addCommon_initNvData();
}

/*********************************************************************
 * @fn      addCommon_initNvItemID
 *
 * @brief   Initial common non-volatile flash item ID.
 *
 * @param   none
 *
 * @return  none
 */
static void addCommon_initNvItemID(void)
{
	// no operation
}

/*********************************************************************
 * @fn      addCommon_initNvData
 *
 * @brief   Common initial non-volatile flash data.
 *
 * @param   none
 *
 * @return  none
 */
static void addCommon_initNvData(void)
{
	// no operation
}

/*********************************************************************
 * @fn      addCommon_eventLoop
 *
 * @brief   Common Event Loop Processor for zclGeneral.
 *
 * @param   uint8 task_id, uint16 events
 *
 * @return  none
 */
uint16 addCommon_eventLoop( uint8 task_id, uint16 events )
{
	if (events & AC_EVENT_DEVICE_LED_NETWORK_STATE)
	{
		//AC_DEVICE_LED_NETWORK_STATE += 1;//LED灯反转，常亮或常灭
		addCommon_ledFlash(1, 100);//闪灯
		osal_start_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_LED_NETWORK_STATE, AC_DELAY_LED_NETWORK_STATE);
		return ( events ^ AC_EVENT_DEVICE_LED_NETWORK_STATE);
	}
	
	if (events & AC_EVENT_DEVICE_KEY_TIMING)
	{
		addCommon_handleKeyEvent();
		return ( events ^ AC_EVENT_DEVICE_KEY_TIMING );
	}

	return 0;
}

/*********************************************************************
 * @fn      addCommon_polling
 *
 * @brief   Common polling.
 *
 * @param   none
 *
 * @return  none
 */
void addCommon_polling(void)
{	
	if (acDelayPowerOn > 0)
	{
		acDelayPowerOn--;
	} else 
	{
		// no operation
	}
	
	if (acDelayJoinNetwork > 0)//两分钟后，置0
	{
		acDelayJoinNetwork--;
		addCommon_ledFlash(1, 100);
	} else 
	{
		// no operation		
		AC_DEVICE_LED_NETWORK_STATE = 0;
	}
}

/*********************************************************************
 * @fn      addCommon_handleOneTime
 *
 * @brief   Common handle one time.
 *
 * @param   none
 *
 * @return  none
 */
void addCommon_handleOneTime(void)
{
	// setup key	
	if (acDeviceSetupKeyStatus == AC_KEY_DOWN) // long press
	{
		//addCommon_clearKeyData();
		acDeviceSetupKeyStatus = AC_KEY_DOWN;
	} else if (acDeviceSetupKeyStatus == AC_KEY_UP_600MS_TO_1S)
	{
		//addCommon_clearKeyData();
		acDeviceSetupKeyStatus = AC_KEY_NORMAL;
		if (acDoubleClickCounter_whenKeyUp <= 1) // short press
		{
			// no operation
		} else // double click
		{
			// no operation
		}
	} else if (acDeviceSetupKeyStatus == AC_KEY_UP_1S_TO_5S)
	{
		//addCommon_clearKeyData();
		acDeviceSetupKeyStatus = AC_KEY_NORMAL;
		// no operation
	} else if ((acDeviceSetupKeyStatus == AC_KEY_UP_5S_TO_10S) || (acDeviceSetupKeyStatus == AC_KEY_UP_ABOVE_10S))
	{
		//addCommon_clearKeyData();
		acDeviceSetupKeyStatus = AC_KEY_NORMAL;	
		acDoubleClickCounter_whenKeyUp = 0;
		addCommon_factoryReset();
	}
	
	else // others
		
	{
		//addCommon_clearKeyData();
		acDeviceSetupKeyStatus = AC_KEY_NORMAL;
	}
	
	//acDoubleClickCounter_whenKeyUp = 0;
}

/*********************************************************************
 * @fn      addCommon_isrPort0
 *
 * @brief   Port0 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addCommon_isrPort0(void)
{
#if defined(HOME_GATEWAY)
	addHomeGateway_isrPort0();
#elif defined(DIMMER_SWITCH) || defined(DIMMER_SWITCH_TOUCHKEY)
	addDimmerSwitch_isrPort0();
#elif defined(SCENE_SELECTOR) || defined(SCENE_SELECTOR_TOUCHKEY)
	addSceneSelector_isrPort0();
#elif defined(DIMMER_LIGHT)
	addDimmerLight_isrPort0();
#else
#endif
}

/*********************************************************************
 * @fn      addCommon_isrPort1
 *
 * @brief   Port1 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addCommon_isrPort1(void)
{
	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P1INT_VECTOR    
		//__interrupt void P1_ISR(void) 
		{
			// P1_2(key setup)
			if ((P1IFG & 0x04) == 0x04) 
			{
				if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(AC_DEVICE_SETUP_KEY == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(AC_DEVICE_SETUP_KEY == 0))
					{
						acDeviceSetupKeyStatus = addCommon_handleKeyDown();
						P1IFG &= ~0x04; // clear interrupt flag
						P1IF = 0;
						EA = 0;
						PICTL &= ~0x06;
						acInterruptTriggerEdgePort1 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(AC_DEVICE_SETUP_KEY == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(AC_DEVICE_SETUP_KEY == 1))
					{
						acDeviceSetupKeyStatus = addCommon_handleKeyUp();
						if (acDeviceSetupKeyStatus == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								acDeviceSetupKeyStatus = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								acDeviceSetupKeyStatus = AC_KEY_UP_600MS_TO_1S;
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} else 
						{
							osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
						}
						
						P1IFG &= ~0x04; // clear interrupt flag
						P1IF = 0;
						EA = 0;
						PICTL |= 0x06;
						acInterruptTriggerEdgePort1 = AC_INTERRUPT_TRIGGER_EDGE_FALL;
						EA = 1;
					}
				}
			}
		}
	} else 
	{
		P1IFG &= ~0x04; // clear interrupt flag for P1_2
		P1IF = 0;
	}

#if defined(HOME_GATEWAY)
	addHomeGateway_isrPort1();		
#elif defined(DIMMER_SWITCH) || defined(DIMMER_SWITCH_TOUCHKEY)
	addDimmerSwitch_isrPort1();  
#elif defined(SCENE_SELECTOR) || defined(SCENE_SELECTOR_TOUCHKEY)
	addSceneSelector_isrPort1();
#elif defined(DIMMER_LIGHT)	
	addDimmerLight_isrPort1();	  
#else			
#endif
}

/*********************************************************************
 * @fn      addCommon_isrPort2
 *
 * @brief   Port2 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addCommon_isrPort2(void)
{
#if defined(HOME_GATEWAY)
	addHomeGateway_isrPort2();
#elif defined(DIMMER_SWITCH) || defined(DIMMER_SWITCH_TOUCHKEY)
	addDimmerSwitch_isrPort2();
#elif defined(SCENE_SELECTOR) || defined(SCENE_SELECTOR_TOUCHKEY)	
	//if (acDelayPowerOn == 0)
	{
		addSceneSelector_isrPort2();
	}
#elif defined(DIMMER_LIGHT)	
	addDimmerLight_isrPort2();
#else			
#endif
}

/*********************************************************************
 * @fn      addCommon_delayMS
 *
 * @brief   Delay.
 *
 * @param   delayMS
 *
 * @return  none
 */
void addCommon_delayMS(uint16 delayMS) // 65536 = 1024 * 64
{
	uint16 lFactor = delayMS >> 6;
	uint16 lRemainder = delayMS & 0x3f;

	for (uint16 i = 0; i < lFactor; i++)
	{
		MicroWait(64000);
	}
	MicroWait(lRemainder * 1000);
}

/*********************************************************************
 * @fn      addCommon_ledFlash
 *
 * @brief   Led flash.
 *
 * @param   uint8 flashCount, uint16 flashDelay
 *
 * @return  void
 */
void addCommon_ledFlash(uint8 flashCount, uint16 flashDelay)
{
	AC_DEVICE_LED_NETWORK_STATE = 0;
	addCommon_delayMS(flashDelay);
	
	for (int i = 0; i < flashCount; i++)
	{
		AC_DEVICE_LED_NETWORK_STATE = 1;
		addCommon_delayMS(flashDelay);
		AC_DEVICE_LED_NETWORK_STATE = 0;
		addCommon_delayMS(flashDelay);
	}
}

/*********************************************************************
 * @fn      addCommon_atNormalWorkState
 *
 * @brief   If get into normal work state.
 *
 * @param   bool judgeNetworkState
 *
 * @return  bool
 */
bool addCommon_atNormalWorkState(bool judgeNetworkState)
{
	if (acDelayPowerOn == 0)
	{
		if (TRUE == judgeNetworkState)
		{
			if ((*acPointer_NwkState == DEV_END_DEVICE) || // Started as device after authentication
				(*acPointer_NwkState == DEV_ROUTER) || // Device joined, authenticated and is a router
				(*acPointer_NwkState == DEV_ZB_COORD)) // Started as Zigbee Coordinator
			{
				return TRUE;
			} else { return FALSE; }
		} else { return TRUE; }
	} else { return FALSE;	}	
}

/*********************************************************************
 * @fn      addCommon_networkState_led
 *
 * @brief  Network state led.
 *
 * @param   devStates_t nwkState
 *
 * @return  none
 */
void addCommon_networkState_led(devStates_t nwkState)
{
	if ((nwkState == DEV_HOLD) || // Initialized - not started automatically
		(nwkState == DEV_INIT) || // Initialized - not connected to anything
		(nwkState == DEV_COORD_STARTING)) // Started as Zigbee Coordinator		
	{
		osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_LED_NETWORK_STATE);
		AC_DEVICE_LED_NETWORK_STATE = 1;//用此状态开灯＝＝长亮
		acNwkState_stopFlash = TRUE;
	} else if ((nwkState == DEV_NWK_DISC) || // Discovering PAN's to join
		(nwkState == DEV_NWK_JOINING) || // Joining a PAN
		(nwkState == DEV_NWK_BACKOFF) || // Device is waiting before trying to rejoin
		(nwkState == DEV_NWK_SEC_REJOIN_CURR_CHANNEL) || // ReJoining a PAN in secure mode scanning in current channel, only for end devices
		(nwkState == DEV_NWK_SEC_REJOIN_ALL_CHANNEL) || // ReJoining a PAN in secure mode scanning in all channels, only for end devices
		(nwkState == DEV_NWK_TC_REJOIN_CURR_CHANNEL) || // ReJoining a PAN in Trust center mode scanning in current channel, only for end devices
		(nwkState == DEV_NWK_TC_REJOIN_ALL_CHANNEL) || // ReJoining a PAN in Trust center mode scanning in all channels, only for end devices
		(nwkState == DEV_END_DEVICE_UNAUTH) || // Joined but not yet authenticated by trust center
		(nwkState == DEV_NWK_KA) || // Device is sending KeepAlive message to its parent
		(nwkState == DEV_NWK_ORPHAN)) // Device has lost information about its parent
	{
		if (acNwkState_stopFlash == TRUE)
		{
			osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_LED_NETWORK_STATE);
			osal_set_event(*acPointer_TaskID, AC_EVENT_DEVICE_LED_NETWORK_STATE);//状态灯闪1S开关闪灯
			acNwkState_stopFlash = FALSE;
		}
	} else if ((nwkState == DEV_END_DEVICE) || // Started as device after authentication
		(nwkState == DEV_ROUTER) || // Device joined, authenticated and is a router
		(nwkState == DEV_ZB_COORD)) // Started as Zigbee Coordinator
	{
		osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_LED_NETWORK_STATE);
		AC_DEVICE_LED_NETWORK_STATE = 0;//用此状态关灯
		acNwkState_stopFlash = TRUE;
		
#if defined(POWER_SAVING)
		if (acSleepFlag == FALSE)
		{
			//addCommon_sleep();
			acDelayJoinNetwork = AC_DELAY_DEEP_SLEEP_AFTER_JOIN_NETWORK;
			osal_start_timerEx(*acPointer_TaskID, AC_EVENT_POLLING, AC_DELAY_POLLING);
		}
#else
#endif
	}
}

/*********************************************************************
 * @fn      addCommon_handleKeyEvent
 *
 * @brief   Setup key down.
 *
 * @param   void
 *
 * @return  void
 */
static void addCommon_handleKeyEvent(void)
{	
	if (acDeviceKeyTimingCounter < 150) // max 15000ms
	{
		acDeviceKeyTimingCounter++;
		if (acDeviceKeyTimingCounter == 1) // 100ms
		{
			addCommon_ledFlash(1, 100);
		} else if (acDeviceKeyTimingCounter == 6) // 600ms
		{
			//addCommon_ledFlash(2, 100);
		} else if (acDeviceKeyTimingCounter == 10) // 1000ms
		{
			//addCommon_ledFlash(2, 100);
#if defined(DIMMER_SWITCH) || defined(DIMMER_SWITCH_TOUCHKEY)
			addDimmerSwitch_sendLevelControlMove();
#elif defined(SCENE_SELECTOR) || defined(SCENE_SELECTOR_TOUCHKEY)
			addSceneSelector_sendLevelControlMove();
#else
#endif
		} else if (acDeviceKeyTimingCounter == 50) // 5000ms
		{
			addCommon_ledFlash(3, 200);
#if defined(DIMMER_SWITCH) || defined(DIMMER_SWITCH_TOUCHKEY)
			addDimmerSwitch_factoryResetCheck();
			addDimmerSwitch_wakeupCheck();
#elif defined(SCENE_SELECTOR) || defined(SCENE_SELECTOR_TOUCHKEY)
			addSceneSelector_factoryResetCheck();
			addSceneSelector_wakeupCheck();
#else
#endif
		} else if (acDeviceKeyTimingCounter == 100) // 10000ms
		{
			addCommon_ledFlash(5, 200);
#if defined(DIMMER_SWITCH) || defined(DIMMER_SWITCH_TOUCHKEY)
			addDimmerSwitch_factoryResetCheck();
			addDimmerSwitch_wakeupCheck();
#elif defined(SCENE_SELECTOR) || defined(SCENE_SELECTOR_TOUCHKEY)
			addSceneSelector_factoryResetCheck();
			addSceneSelector_wakeupCheck();
#else
#endif
		}
		osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_KEY_TIMING);
		osal_start_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_KEY_TIMING, acDeviceKeyTimingUnit); // 100ms
	} else 
	{
		osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_KEY_TIMING);
	}
}

/*********************************************************************
 * @fn      addCommon_handleKeyDown
 *
 * @brief   Key down.
 *
 * @param   void
 *
 * @return  uint8
 */
uint8 addCommon_handleKeyDown(void)
{
	if (acDoubleClickCheckFlag == FALSE)
	{

		osal_start_timerEx(*acPointer_TaskID, AC_EVENT_DOUBLE_CLICK_CHECK, AC_DELAY_DOUBLE_CLICK);
		acDoubleClickCheckFlag = TRUE;
		acDoubleClickCounter_whenKeyUp = 0;
		acDoubleClickCounter_whenKeyDown++;
	}
	
	osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_KEY_TIMING);
	acDeviceKeyTimingCounter = 0;
	osal_set_event(*acPointer_TaskID, AC_EVENT_DEVICE_KEY_TIMING);
	
	return AC_KEY_DOWN;
}

/*********************************************************************
 * @fn      addCommon_handleKeyUp
 *
 * @brief   Key up.
 *
 * @param   void
 *
 * @return  uint8
 */
uint8 addCommon_handleKeyUp(void)
{
	//osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_KEY_TIMING);
	if ((acDeviceKeyTimingCounter >= 1) && (acDeviceKeyTimingCounter < 6)) // >= 100ms && < 600ms
	{
		return AC_KEY_UP_BELOW_600MS;
	} else if ((acDeviceKeyTimingCounter >= 6) && (acDeviceKeyTimingCounter < 10)) // >= 600ms && < 1000ms
	{
		return AC_KEY_UP_600MS_TO_1S;
	} else if ((acDeviceKeyTimingCounter >= 10) && (acDeviceKeyTimingCounter < 50)) // >= 1000ms && < 5000ms
	{
		return AC_KEY_UP_1S_TO_5S;
	} else if ((acDeviceKeyTimingCounter >= 50) && (acDeviceKeyTimingCounter < 100)) // >= 5000ms && < 10000ms
	{
		return AC_KEY_UP_5S_TO_10S;
	} else if (acDeviceKeyTimingCounter >= 100) // >= 10000ms
	{
		return AC_KEY_UP_ABOVE_10S;
	} else 
	{
		return AC_KEY_NORMAL;
	}
	acDeviceKeyTimingCounter = 0;
}

/**************************************************************************************************
 * @fn          NLME_SetRejoinPollRate
 *
 * @brief       
 *
 * input parameters
 *
 * @param       newRate = number of milliseconds to do next poll.
 *                        0 will turn off the polling.
 *                        1 will do a one time poll.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void NLME_SetRejoinPollRate(uint32 newRate)
{
  osal_nv_write(ZCD_NV_REJOIN_POLL_RATE, 0, 2, (uint8 *)(&newRate));
}

/*********************************************************************
 * @fn      addCommon_sleep
 *
 * @brief   Sleep.
 *
 * @param   void
 *
 * @return  void
 */
void addCommon_sleep(void)
{
#if defined(POWER_SAVING)
	acSleepFlag = TRUE;			// wakeUp时候会设置为FALSE

	addCommon_ledFlash(7, 100);

	osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_LED_NETWORK_STATE);
	AC_DEVICE_LED_NETWORK_STATE = 0;
	osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_POLLING);

	osal_pwrmgr_device(PWRMGR_BATTERY);
	osal_pwrmgr_task_state(*acPointer_TaskID, PWRMGR_CONSERVE);
	
	NLME_SetPollRate(600000); 		//10分钟poll一次父节点
	//zgPollRate = 0;
	NLME_SetQueuedPollRate(0);
	//zgQueuedPollRate = 0;
	NLME_SetResponseRate(0);
	//zgResponsePollRate = 0;
	//NLME_SetRejoinPollRate(0);
	//zgRejoinPollRate = 0;
	
#else
#endif
}

/*********************************************************************
 * @fn      addCommon_wakeup
 *
 * @brief   Wakeup.
 *
 * @param   void
 *
 * @return  void
 */
void addCommon_wakeup(void)
{
#if defined(POWER_SAVING)
	acSleepFlag = FALSE;

	osal_pwrmgr_device(PWRMGR_ALWAYS_ON);
	osal_pwrmgr_task_state(*acPointer_TaskID, PWRMGR_HOLD);

	NLME_SetPollRate(500);
	//zgPollRate = 1000;
	NLME_SetQueuedPollRate(100);
	//zgQueuedPollRate = 100;
	NLME_SetResponseRate(100);
	//zgResponsePollRate = 100;
	//NLME_SetRejoinPollRate(440);
	//zgRejoinPollRate = 440;
	
	acDelayJoinNetwork = AC_DELAY_JOIN_NETWORK;
	osal_start_timerEx(*acPointer_TaskID, AC_EVENT_POLLING, AC_DELAY_POLLING);
	
#else
#endif
}

/*********************************************************************
 * @fn		addCommon_factoryReset
 *
 * @brief	Callback from the ZCL General Cluster Library
 *			to set all the Basic Cluster attributes to	default values.
 *
 * @param	none
 *
 * @return	none
 */
void addCommon_factoryReset(void)
{
	uint8 tempProcessCmd[] = {1, 0, 0, 0}; // used for reset command, { length + cmd0 + cmd1 + data }

	// Put device back to factory default settings
	zgWriteStartupOptions(ZG_STARTUP_SET, 3);	// bit set both default configuration and default network

	// restart device
	MT_SysCommandProcessing(tempProcessCmd);
}

