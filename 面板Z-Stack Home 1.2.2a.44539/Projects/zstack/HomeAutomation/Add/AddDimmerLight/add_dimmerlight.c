/*********************************************************************
 * INCLUDES
 */
#include "add_dimmerlight.h"

/*********************************************************************
 * LOCAL VARIABLES
 */
byte aDimmerLightCW = 127;
byte aDimmerLightWW = 127;
byte aDimmerLightLeaveNetworkCount = 0;

volatile bool handleOneTimeFinished = TRUE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void addDimmerLight_init(void);
static void addDimmerLight_initHal(void);
static void addDimmerLight_initAl(void);
static void addDimmerLight_initNvItemID(void);
static void addDimmerLight_initNvData(void);
uint16 addDimmerLight_eventLoop(uint8 task_id, uint16 events);
static void addDimmerLight_polling(void);
static void addDimmerLight_handleOneTime(void);
void addDimmerLight_isrPort0(void);
void addDimmerLight_isrPort1(void);
void addDimmerLight_isrPort2(void);

/*********************************************************************
 * @fn      addDimmerLight_init
 *
 * @brief   Dimmer light initial.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerLight_init(void)
{
	addCommon_init();

	addDimmerLight_initHal();
	addCommon_delayMS(500);
	addDimmerLight_initAl();

	osal_nv_read(A_DIMMERLIGHT_NV_ITEM_ID_LEAVE_NETWORK_COUNT, 0, 1, &aDimmerLightLeaveNetworkCount);
	aDimmerLightLeaveNetworkCount++;
	if (aDimmerLightLeaveNetworkCount >= 5)
	{
		aDimmerLightLeaveNetworkCount = 0;
		osal_nv_write(A_DIMMERLIGHT_NV_ITEM_ID_LEAVE_NETWORK_COUNT, 0, 1, &aDimmerLightLeaveNetworkCount);
		addCommon_delayMS(300);

		T3CC0 = 255;
		addCommon_delayMS(300);
		T3CC0 = 0;
		addCommon_delayMS(300);
		T3CC0 = 255;
		addCommon_delayMS(300);
		T3CC0 = 0;
		addCommon_delayMS(300);
		T3CC0 = 255;
		addCommon_delayMS(300);
		T3CC0 = 0;
		addCommon_delayMS(300);

		zclDimmerLight_factoryReset();
	} else 
	{		
		osal_nv_write(A_DIMMERLIGHT_NV_ITEM_ID_LEAVE_NETWORK_COUNT, 0, 1, &aDimmerLightLeaveNetworkCount);
		addCommon_delayMS(300);
	}
	osal_start_timerEx(zclDimmerLight_TaskID, AC_EVENT_DEVICE_LEAVE_NETWORK, AC_DELAY_EXIT_NETWORK_CLEAR_FLAG_INTERVAL);

	acPointer_NwkState = &zclDimmerLight_NwkState;
	acPointer_TaskID = &zclDimmerLight_TaskID;
	osal_start_timerEx(zclDimmerLight_TaskID, AC_EVENT_POLLING, AC_DELAY_POLLING);
}

/*********************************************************************
 * @fn      addDimmerLight_initHal
 *
 * @brief   Dimmer light initial hardware abstract layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerLight_initHal(void)
{		
	 P1DIR |= 0x40; // Cold white on port P1_6
	 P1SEL |= 0x40;
	 P2SEL |= 0x20;
	 PERCFG |= 0x20;
		 
	 //T3CC0 = 127; // 50%
	 //T3CCTL0 = 0x2c;
	 //T3CTL = 0x90;
		 
	 /*P2DIR |= 0x01; // Warm white on port P2_0
	 P2SEL |= 0x01;
	 P2SEL |= 0x10;
	 PERCFG |= 0x10;
	
	 //T4CC0 = 127; // 50%
	 //T4CCTL0 = 0x2c;
	 //T4CTL = 0x90;*/
}

/*********************************************************************
 * @fn      addDimmerLight_initAl
 *
 * @brief   Dimmer light initial application layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerLight_initAl(void)
{	
	addDimmerLight_initNvItemID();
	addDimmerLight_initNvData();
			
	osal_nv_read(A_DIMMERLIGHT_NV_ITEM_ID_CW, 0, 1, &aDimmerLightCW);
	T3CC0 = A_DIMMERLIGHT_LEVEL_MAX - aDimmerLightCW;
	T3CCTL0 = 0x2c;
	T3CTL = 0x90;
	
	/*osal_nv_read(A_DIMMERLIGHT_NV_ITEM_ID_WW, 0, 1, &aDimmerLightWW);
	T4CC0 = A_DIMMERLIGHT_LEVEL_MAX - aDimmerLightCW;
	T4CCTL0 = 0x2c;
	T4CTL = 0x90;*/
}

/*********************************************************************
 * @fn      addDimmerLight_initNvItemID
 *
 * @brief   Dimmer light initial non-volatile flash item ID.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerLight_initNvItemID(void)
{
	osal_nv_item_init(A_DIMMERLIGHT_NV_ITEM_ID_CW, 1, &aDimmerLightCW);
	osal_nv_item_init(A_DIMMERLIGHT_NV_ITEM_ID_WW, 1, &aDimmerLightWW);
	osal_nv_item_init(A_DIMMERLIGHT_NV_ITEM_ID_LEAVE_NETWORK_COUNT, 1, &aDimmerLightLeaveNetworkCount);
}

/*********************************************************************
 * @fn      addDimmerLight_initNvData
 *
 * @brief   Dimmer light initial non-volatile flash data.
 *
 * @param   none
 *
 * @return  none
 */
 static void addDimmerLight_initNvData(void)
{
	// no operation
}

/*********************************************************************
 * @fn      addDimmerLight_eventLoop
 *
 * @brief   Dimmer light Event Loop Processor for zclGeneral.
 *
 * @param   uint8 task_id, uint16 events
 *
 * @return  uint16
 */
uint16 addDimmerLight_eventLoop(uint8 task_id, uint16 events)
{		
	if (events & AC_EVENT_POLLING)
	{
		addDimmerLight_polling();
		//osal_start_timerEx(zclDimmerLight_TaskID, AC_EVENT_POLLING, AC_DELAY_POLLING);
		if (acDelayJoinNetwork != 0)
		{
			osal_start_timerEx(zclDimmerLight_TaskID, AC_EVENT_POLLING, AC_DELAY_POLLING);
		}
		return ( events ^ AC_EVENT_POLLING );
	}
	
	if (events & AC_EVENT_DEVICE_LEAVE_NETWORK)
	{	
		aDimmerLightLeaveNetworkCount = 0;
		osal_nv_write(A_DIMMERLIGHT_NV_ITEM_ID_LEAVE_NETWORK_COUNT, 0, 1, &aDimmerLightLeaveNetworkCount);
		addCommon_delayMS(300);
		
		return ( events ^ AC_EVENT_DEVICE_LEAVE_NETWORK);
	}
	
	if (events & AC_EVENT_HANDLE_ONE_TIME)
	{			
		addDimmerLight_handleOneTime();
		handleOneTimeFinished = TRUE;
		return ( events ^ AC_EVENT_HANDLE_ONE_TIME);
	}
	
	return addCommon_eventLoop(task_id, events);
}

/*********************************************************************
 * @fn      addDimmerLight_polling
 *
 * @brief   Dimmer light.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerLight_polling(void)
{
	addCommon_polling();
}

/*********************************************************************
 * @fn      addDimmerLight_handleOneTime
 *
 * @brief   Dimmer light handle one time.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerLight_handleOneTime(void)
{
	uint8 bTemp = 0;

handleOneTimeFinished = FALSE;

	addCommon_handleOneTime();
}

/*********************************************************************
 * @fn      addDimmerLight_isrPort0
 *
 * @brief   Port0 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerLight_isrPort0(void)
{
	bool handleOneTime = FALSE;
	
	IEN1 &= ~0x20; // port0 interrupt disable

	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P0INT_VECTOR    
		//__interrupt void P0_ISR(void) 
		{
			if (handleOneTime == TRUE)
			{
				handleOneTime = FALSE;
				osal_set_event(zclDimmerLight_TaskID, AC_EVENT_HANDLE_ONE_TIME);
				//addDimmerLight_handleOneTime();
			}
		}
	} else 
	{
		//P0IFG &= ~0x70; // clear interrupt flag
		//P0IF = 0;
	}	
	
	IEN1 |= 0x20; // port0 interrupt enable
}

/*********************************************************************
 * @fn      addDimmerLight_isrPort1
 *
 * @brief   Port1 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerLight_isrPort1(void)
{
	bool handleOneTime = FALSE;
	
	IEN2 &= ~0x10; // port1 interrupt disable
	
	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P1INT_VECTOR    
		//__interrupt void P1_ISR(void) 
		{
			if (handleOneTime == TRUE)
			{
				handleOneTime = FALSE;
				osal_set_event(zclDimmerLight_TaskID, AC_EVENT_HANDLE_ONE_TIME);
				//addDimmerLight_handleOneTime();
			}
		}
	} else 
	{
		//P1IFG &= ~0xc0; // clear interrupt flag
		//P1IF = 0;
	}
	
	IEN2 |= 0x10; // port1 interrupt enable
}

/*********************************************************************
 * @fn      addDimmerLight_isrPort2
 *
 * @brief   Port2 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerLight_isrPort2(void)
{
	bool handleOneTime = FALSE;
	
	IEN2 &= ~0x02; // port2 interrupt disable
	
	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P2INT_VECTOR    
		//__interrupt void P2_ISR(void) 
		{
			if (handleOneTime == TRUE)
			{
				handleOneTime = FALSE;
				osal_set_event(zclDimmerLight_TaskID, AC_EVENT_HANDLE_ONE_TIME);
				//addDimmerLight_handleOneTime();
			}
		} 
	} else 
	{
		//P2IFG &= ~0x01; // clear interrupt flag
		//P2IF = 0;
	}	
	
	IEN2 |= 0x02; // port2 interrupt enable
}

