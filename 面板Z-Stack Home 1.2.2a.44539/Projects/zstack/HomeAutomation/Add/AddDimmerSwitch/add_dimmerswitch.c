/*********************************************************************
 * INCLUDES
 */
#include "add_dimmerswitch.h"

/*********************************************************************
 * LOCAL VARIABLES
 */
volatile byte aDimmerSwitchKeyOn = AC_KEY_NORMAL;
volatile byte aDimmerSwitchKeyOff = AC_KEY_NORMAL;
volatile byte aDimmerSwitchKeyDimmingUp = AC_KEY_NORMAL;
volatile byte aDimmerSwitchKeyDimmingDown = AC_KEY_NORMAL;
volatile byte aDimmerSwitchKeyCold = AC_KEY_NORMAL;
volatile byte aDimmerSwitchKeyWarm = AC_KEY_NORMAL;

uint16 aDimmerSwitchEndpoint_GroupID = 1;

volatile uint8 aDimmerSwitchFactoryReset = ADD_DIMMERSWITCH_FACTORYRESET_KEYNORMAL;
volatile uint8 aDimmerSwitchWakeup = ADD_DIMMERSWITCH_WAKEUP_KEYNORMAL;

uint16 colorTemperature = 261;
uint16 stepSize = 20;
uint16 transTime = 1;

#if defined(DIMMER_SWITCH)
#elif defined(DIMMER_SWITCH_TOUCHKEY)

uint8 aDimmerSwitchApt8l08_i2cWaitStatus;

const uint8 aDimmerSwitchApt8l08_registerAddress[16] = {
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_GSR,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_MCON,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_FILTER0,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_K0_ENB,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_K1_ENB,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_SENSE_CON,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_SENSE_S,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_GSR_K07,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_GSR_K17,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_REF_UTH,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_KEY_ATH,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_DSMIT,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_MCONH,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_FILTER1,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_K0_CON,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_K1_CON
};

const uint8 aDimmerSwitchApt8l08_registerData[16] = {
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_GSR,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_MCON,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_FILTER0,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_K0_ENB,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_K1_ENB,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_SENSE_CON,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_SENSE_S,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_GSR_K07,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_GSR_K17,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_REF_UTH,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_KEY_ATH,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_DSMIT,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_MCONH,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_FILTER1,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_K0_CON,
	ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_K1_CON
};

const uint8 aDimmerSwitchApt8l08_tkFin8bit[16] 
	= {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
volatile uint16 aDimmerSwitchApt8l08_16bitKeyMap = 0;
volatile uint16 aDimmerSwitchApt8l08_16bitKeyMap_saved = 0;

#else
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void addDimmerSwitch_init(void);
static void addDimmerSwitch_initHal(void);
static void addDimmerSwitch_initAl(void);
static void addDimmerSwitch_initNvItemID(void);
static void addDimmerSwitch_initNvData(void);
uint16 addDimmerSwitch_eventLoop(uint8 task_id, uint16 events);
static void addDimmerSwitch_polling(void);
static void addDimmerSwitch_handleOneTime(void);
void addDimmerSwitch_isrPort0(void);
void addDimmerSwitch_isrPort1(void);
void addDimmerSwitch_isrPort2(void);

static void addDimmerSwitch_sendDestinationAddressConfigure(uint8 endPoint);
static void addDimmerSwitch_zclLighting_ColorControl_Send_StepColorTemperatureCmd_Up(uint8 endPoint);
static void addDimmerSwitch_zclLighting_ColorControl_Send_StepColorTemperatureCmd_Down(uint8 endPoint);
void addDimmerSwitch_sendLevelControlMove(void);
static void addDimmerSwitch_clearKeyData(void);
void addDimmerSwitch_factoryResetCheck(void);
void addDimmerSwitch_wakeupCheck(void);
void addDimmerSwitch_setGroupID(uint8 endPoint, aps_Group_t *group);

#if defined(DIMMER_SWITCH)	
#elif defined(DIMMER_SWITCH_TOUCHKEY)

void addDimmerSwitch_APT8L08_init(void);
static void addDimmerSwitch_APT8L08_initHal(void);
static void addDimmerSwitch_APT8L08_initAl(void);
static void addDimmerSwitch_APT8L08_I2C_start(void);
static void addDimmerSwitch_APT8L08_I2C_stop(void);
static void addDimmerSwitch_APT8L08_I2C_waitACK(void);
static void addDimmerSwitch_APT8L08_I2C_noACK(void);
static void addDimmerSwitch_APT8L08_I2C_ACK(void);
static void addDimmerSwitch_APT8L08_I2C_write8bit(uint8 dataWrite);
static uint8 addDimmerSwitch_APT8L08_I2C_read8bit(void);
static void addDimmerSwitch_APT8L08_I2C_writeByte(uint8 address, uint8 dataWrite);
static uint8 addDimmerSwitch_APT8L08_I2C_readByte(uint8 address);
static void addDimmerSwitch_APT8L08_I2C_writeRegister(uint8 address, uint8 dataWrite);
	
#else
#endif

/*********************************************************************
 * @fn      addDimmerSwitch_init
 *
 * @brief   Dimmer switch initial.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerSwitch_init(void)
{
	addCommon_init();
	
#if defined(DIMMER_SWITCH)
	addDimmerSwitch_initHal();
	addCommon_delayMS(500);
	addDimmerSwitch_initAl();
	
#elif defined(DIMMER_SWITCH_TOUCHKEY)
	addDimmerSwitch_APT8L08_init();
	
#else
#endif

	acPointer_NwkState = &zclDimmerSwitch_NwkState;
	acPointer_TaskID = &zclDimmerSwitch_TaskID;

	addCommon_wakeup();
}

/*********************************************************************
 * @fn      addDimmerSwitch_initHal
 *
 * @brief   Dimmer switch initial hardware abstract layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_initHal(void)
{
	P2SEL &= ~0x01; // P2_0(key dimming down)
	P2DIR &= ~0x01;
	P2INP &= ~0x80; // port2 all pull up
	P2INP &= ~0x01;
	
	APCFG &= ~0x31; // P0_4(key on) P0_5(key off) P0_0(key dimming up)
	P0SEL &= ~0x31;
	P0DIR &= ~0x31;
	P2INP &= ~0x20; // port0 all pull up
	P0INP &= ~0x31;
	
	P1SEL &= ~0xc0; // P1_6(key cold) P1_7(key warm)
	P1DIR &= ~0xc0;
	P2INP &= ~0x40; // port1 all pull up
	P1INP &= ~0xc0;

	EA = 0;
	
	MicroWait(50000);
	MicroWait(50000);

	P0IEN |= 0x31; // P0_4 P0_5 P0_0 interrupt enable
	PICTL |= 0x01; // fall edge
	P0IFG &= ~0x31; // clear interrupt flag
	IEN1 |= 0x20; // port0 interrupt enable

	P1IEN |= 0xc0; // P1_6 P1_7 interrupt enable
	PICTL |= 0x06; // fall edge
	P1IFG &= ~0xc0; // clear interrupt flag
	IEN2 |= 0x10; // port1 interrupt enable

	P2IEN |= 0x01; // P2_0 interrupt enable
	PICTL |= 0x08; // fall edge
	P2IFG &= ~0x01; // clear interrupt flag
	IEN2 |= 0x02; // port2 interrupt enable
	
	IP0 |= (1 << 5); // set port0 interrupt highest priority
	IP1 |= (1 << 5);
	
	IP0 |= (1 << 4); // set port1 interrupt highest priority
	IP1 |= (1 << 4);
	
	IP0 |= (1 << 1); // set port2 interrupt highest priority
	IP1 |= (1 << 1);
	
	EA = 1;
}

/*********************************************************************
 * @fn      addDimmerSwitch_initAl
 *
 * @brief   Dimmer switch initial application layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_initAl(void)
{	
	addDimmerSwitch_initNvItemID();
	addDimmerSwitch_initNvData();
}

/*********************************************************************
 * @fn      addDimmerSwitch_initNvItemID
 *
 * @brief   Dimmer switch initial non-volatile flash item ID.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_initNvItemID(void)
{
	osal_nv_item_init(ADD_DIMMERSWITCH_NV_ITEM_ID_ENDPOINT_GROUPID, 2, &aDimmerSwitchEndpoint_GroupID);
}

/*********************************************************************
 * @fn      addDimmerSwitch_initNvData
 *
 * @brief   Dimmer switch initial non-volatile flash data.
 *
 * @param   none
 *
 * @return  none
 */
 static void addDimmerSwitch_initNvData(void)
{
	osal_nv_read(ADD_DIMMERSWITCH_NV_ITEM_ID_ENDPOINT_GROUPID, 0, 2, &aDimmerSwitchEndpoint_GroupID);
}

/*********************************************************************
 * @fn      addDimmerSwitch_eventLoop
 *
 * @brief   Dimmer switch Event Loop Processor for zclGeneral.
 *
 * @param   uint8 task_id, uint16 events
 *
 * @return  uint16
 */
uint16 addDimmerSwitch_eventLoop(uint8 task_id, uint16 events)
{		
	if (events & AC_EVENT_POLLING)
	{
		addDimmerSwitch_polling();
		
		if (acDelayJoinNetwork != 0)
		{
			osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_POLLING);
			osal_start_timerEx(*acPointer_TaskID, AC_EVENT_POLLING, AC_DELAY_POLLING);
		} else
		{
#if defined(POWER_SAVING)
			if (acSleepFlag == FALSE)
			{
				addCommon_sleep();
			}
#else
#endif
		}
		return ( events ^ AC_EVENT_POLLING );
	}
	
	if (events & AC_EVENT_HANDLE_ONE_TIME)
	{			
		addDimmerSwitch_handleOneTime();
		return ( events ^ AC_EVENT_HANDLE_ONE_TIME);
	}
	
	if (events & AC_EVENT_DOUBLE_CLICK_CHECK)
	{
		addDimmerSwitch_handleOneTime();
		acDoubleClickCheckFlag = FALSE;
		return ( events ^ AC_EVENT_DOUBLE_CLICK_CHECK);
	}
	
	return addCommon_eventLoop(task_id, events);
}

/*********************************************************************
 * @fn      addDimmerSwitch_polling
 *
 * @brief   Dimmer switch.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_polling(void)
{
	addCommon_polling();
}

/*********************************************************************
 * @fn      addDimmerSwitch_handleOneTime
 *
 * @brief   Dimmer switch handle one time.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerSwitch_handleOneTime(void)
{
	addCommon_handleOneTime();

	// factory reset
	if (aDimmerSwitchFactoryReset == ADD_DIMMERSWITCH_FACTORYRESET_KEYDOWN_10S)
	{
		addDimmerSwitch_clearKeyData();	
		acDoubleClickCounter_whenKeyUp = 0;
		aDimmerSwitchFactoryReset = ADD_DIMMERSWITCH_FACTORYRESET_KEYNORMAL;
		addCommon_factoryReset();
	} 

	else // wakeup

	if (aDimmerSwitchWakeup == ADD_DIMMERSWITCH_WAKEUP_KEYDOWN_10S)
	{
		addDimmerSwitch_clearKeyData();	
		acDoubleClickCounter_whenKeyUp = 0;
		aDimmerSwitchWakeup = ADD_DIMMERSWITCH_WAKEUP_KEYNORMAL;
		addCommon_wakeup();
	}

	else // key on
	
	if (aDimmerSwitchKeyOn == AC_KEY_DOWN) // long press
	{
		addDimmerSwitch_clearKeyData();
		aDimmerSwitchKeyOn = AC_KEY_DOWN;
	} else if ((aDimmerSwitchKeyOn == AC_KEY_UP_600MS_TO_1S) || (aDimmerSwitchKeyOn == AC_KEY_UP_1S_TO_5S)
		|| (aDimmerSwitchKeyOn == AC_KEY_UP_5S_TO_10S) || (aDimmerSwitchKeyOn == AC_KEY_UP_ABOVE_10S))
	{
		addDimmerSwitch_clearKeyData();
		addDimmerSwitch_sendDestinationAddressConfigure(DIMMERSWITCH_ENDPOINT);
		zclGeneral_SendOnOff_CmdOn(DIMMERSWITCH_ENDPOINT, &zclDimmerSwitch_DstAddr, FALSE, 0 );
	}
	
	else // key off
	
	if (aDimmerSwitchKeyOff == AC_KEY_DOWN) // long press
	{
		addDimmerSwitch_clearKeyData();
		aDimmerSwitchKeyOff = AC_KEY_DOWN;
	} else if ((aDimmerSwitchKeyOff == AC_KEY_UP_600MS_TO_1S) || (aDimmerSwitchKeyOff == AC_KEY_UP_1S_TO_5S)
		|| (aDimmerSwitchKeyOff == AC_KEY_UP_5S_TO_10S) || (aDimmerSwitchKeyOff == AC_KEY_UP_ABOVE_10S))
	{
		addDimmerSwitch_clearKeyData();
		addDimmerSwitch_sendDestinationAddressConfigure(DIMMERSWITCH_ENDPOINT);
		zclGeneral_SendOnOff_CmdOff(DIMMERSWITCH_ENDPOINT, &zclDimmerSwitch_DstAddr, FALSE, 0 );
	}

	else // key dimming up
	
	if (aDimmerSwitchKeyDimmingUp == AC_KEY_DOWN) // long press
	{
		addDimmerSwitch_clearKeyData();
		aDimmerSwitchKeyDimmingUp = AC_KEY_DOWN;
	} else if (aDimmerSwitchKeyDimmingUp == AC_KEY_UP_600MS_TO_1S)
	{
		addDimmerSwitch_clearKeyData();
		addDimmerSwitch_sendDestinationAddressConfigure(DIMMERSWITCH_ENDPOINT);
		zclGeneral_SendLevelControlStep(DIMMERSWITCH_ENDPOINT, &zclDimmerSwitch_DstAddr, LEVEL_STEP_UP, 35, 1, FALSE, 0 );
	} else if ((aDimmerSwitchKeyDimmingUp == AC_KEY_UP_1S_TO_5S) || (aDimmerSwitchKeyDimmingUp == AC_KEY_UP_5S_TO_10S) 
		|| (aSceneSelectorKeyDimmingUp == AC_KEY_UP_ABOVE_10S))
	{
		addDimmerSwitch_clearKeyData();
		addDimmerSwitch_sendDestinationAddressConfigure(DIMMERSWITCH_ENDPOINT);
		zclGeneral_SendLevelControlStop(DIMMERSWITCH_ENDPOINT, &zclDimmerSwitch_DstAddr, FALSE, 0 );
	}

	else // key dimming down
	
	if (aDimmerSwitchKeyDimmingDown == AC_KEY_DOWN) // long press
	{
		addDimmerSwitch_clearKeyData();
		aDimmerSwitchKeyDimmingDown = AC_KEY_DOWN;
	} else if (aDimmerSwitchKeyDimmingDown == AC_KEY_UP_600MS_TO_1S)
	{
		addDimmerSwitch_clearKeyData();
		addDimmerSwitch_sendDestinationAddressConfigure(DIMMERSWITCH_ENDPOINT);
		zclGeneral_SendLevelControlStep(DIMMERSWITCH_ENDPOINT, &zclDimmerSwitch_DstAddr, LEVEL_STEP_DOWN, 35, 1, FALSE, 0 );
	} else if ((aDimmerSwitchKeyDimmingDown == AC_KEY_UP_1S_TO_5S) || (aDimmerSwitchKeyDimmingDown == AC_KEY_UP_5S_TO_10S) 
		|| (aDimmerSwitchKeyDimmingDown == AC_KEY_UP_ABOVE_10S))
	{
		addDimmerSwitch_clearKeyData();
		addDimmerSwitch_sendDestinationAddressConfigure(DIMMERSWITCH_ENDPOINT);
		zclGeneral_SendLevelControlStop(DIMMERSWITCH_ENDPOINT, &zclDimmerSwitch_DstAddr, FALSE, 0 );
	}

	else // key cold
	
	if (aDimmerSwitchKeyCold == AC_KEY_DOWN) // long press
	{
		addDimmerSwitch_clearKeyData();
		aDimmerSwitchKeyCold = AC_KEY_DOWN;
	} else if ((aDimmerSwitchKeyCold == AC_KEY_UP_600MS_TO_1S) || (aDimmerSwitchKeyCold == AC_KEY_UP_1S_TO_5S) 
		|| (aDimmerSwitchKeyCold == AC_KEY_UP_5S_TO_10S) || (aSceneSelectorKeyDimmingUp == AC_KEY_UP_ABOVE_10S))
	{
		addDimmerSwitch_clearKeyData();
		addDimmerSwitch_sendDestinationAddressConfigure(DIMMERSWITCH_ENDPOINT);
		addDimmerSwitch_zclLighting_ColorControl_Send_StepColorTemperatureCmd_Up();
	}

	else // key warm
	
	if (aDimmerSwitchKeyWarm == AC_KEY_DOWN) // long press
	{
		addDimmerSwitch_clearKeyData();
		aDimmerSwitchKeyWarm = AC_KEY_DOWN;
	} else if ((aDimmerSwitchKeyWarm == AC_KEY_UP_600MS_TO_1S) || (aDimmerSwitchKeyWarm == AC_KEY_UP_1S_TO_5S) 
		|| (aDimmerSwitchKeyWarm == AC_KEY_UP_5S_TO_10S) || (aDimmerSwitchKeyWarm == AC_KEY_UP_ABOVE_10S))
	{
		addDimmerSwitch_clearKeyData();
		addDimmerSwitch_sendDestinationAddressConfigure(DIMMERSWITCH_ENDPOINT);
		addDimmerSwitch_zclLighting_ColorControl_Send_StepColorTemperatureCmd_Down();
	}
	
	else // others
		
	{
		addDimmerSwitch_clearKeyData();
	}
	
	acDoubleClickCounter_whenKeyUp = 0;
}

/*********************************************************************
 * @fn      addDimmerSwitch_isrPort0
 *
 * @brief   Port0 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerSwitch_isrPort0(void)
{	
#if defined(DIMMER_SWITCH)	
	IEN1 &= ~0x20; // port0 interrupt disable

	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P0INT_VECTOR    
		//__interrupt void P0_ISR(void) 
		{
			// factory reset			
			if (aDimmerSwitchFactoryReset == ADD_DIMMERSWITCH_FACTORYRESET_KEYDOWN_10S)
			{
				osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
			} else if (aDimmerSwitchFactoryReset == ADD_DIMMERSWITCH_FACTORYRESET_KEYDOWN_5S)
			{
				addDimmerSwitch_clearKeyData();
				acDoubleClickCounter_whenKeyUp = 0;
				aDimmerSwitchFactoryReset = ADD_DIMMERSWITCH_FACTORYRESET_KEYNORMAL;
				// no operation
			}

			else // P0_4(key on)
			
			if ((P0IFG & 0x10) == 0x10)
			{
				if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_DIMMERSWITCH_KEY_ON == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_DIMMERSWITCH_KEY_ON == 0))
					{
						aDimmerSwitchKeyOn = addCommon_handleKeyDown();	
						P0IFG &= ~0x10; // clear interrupt flag
						P0IF = 0;
						EA = 0;
						PICTL &= ~0x01;
						acInterruptTriggerEdgePort0 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_DIMMERSWITCH_KEY_ON == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_DIMMERSWITCH_KEY_ON == 1))
					{	
						aDimmerSwitchKeyOn = addCommon_handleKeyUp();
						if (aDimmerSwitchKeyOn == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aDimmerSwitchKeyOn = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aDimmerSwitchKeyOn = AC_KEY_UP_600MS_TO_1S;
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} else 
						{
							osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
						}						
						P0IFG &= ~0x10; // clear interrupt flag
						P0IF = 0;
						EA = 0;
						PICTL |= 0x01;
						acInterruptTriggerEdgePort0 = AC_INTERRUPT_TRIGGER_EDGE_FALL;
						EA = 1;
					}
				}
			}

			else  // P0_5(key off)
			
			if ((P0IFG & 0x20) == 0x20)
			{
				if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_DIMMERSWITCH_KEY_OFF == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_DIMMERSWITCH_KEY_OFF == 0))
					{
						aDimmerSwitchKeyOff = addCommon_handleKeyDown();	
						P0IFG &= ~0x20; // clear interrupt flag
						P0IF = 0;
						EA = 0;
						PICTL &= ~0x01;
						acInterruptTriggerEdgePort0 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_DIMMERSWITCH_KEY_OFF == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_DIMMERSWITCH_KEY_OFF == 1))
					{	
						aDimmerSwitchKeyOff = addCommon_handleKeyUp();
						if (aDimmerSwitchKeyOff == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aDimmerSwitchKeyOff = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aDimmerSwitchKeyOff = AC_KEY_UP_600MS_TO_1S;
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} else 
						{
							osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
						}						
						P0IFG &= ~0x20; // clear interrupt flag
						P0IF = 0;
						EA = 0;
						PICTL |= 0x01;
						acInterruptTriggerEdgePort0 = AC_INTERRUPT_TRIGGER_EDGE_FALL;
						EA = 1;
					}
				}
			}

			else // P0_0(key dimming up)
			
			if ((P0IFG & 0x01) == 0x01) 
			{
				if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_DIMMERSWITCH_KEY_DIMMING_UP == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_DIMMERSWITCH_KEY_DIMMING_UP == 0))
					{
						aDimmerSwitchKeyDimmingUp = addCommon_handleKeyDown();
						P0IFG &= ~0x01; // clear interrupt flag
						P0IF = 0;
						EA = 0;
						PICTL &= ~0x01;
						acInterruptTriggerEdgePort0 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_DIMMERSWITCH_KEY_DIMMING_UP == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_DIMMERSWITCH_KEY_DIMMING_UP == 1))
					{	
						aDimmerSwitchKeyDimmingUp = addCommon_handleKeyUp();
						if (aDimmerSwitchKeyDimmingUp == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aDimmerSwitchKeyDimmingUp = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aDimmerSwitchKeyDimmingUp = AC_KEY_UP_600MS_TO_1S;
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} else 
						{
							osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
						}						
						P0IFG &= ~0x01; // clear interrupt flag
						P0IF = 0;
						EA = 0;
						PICTL |= 0x01;
						acInterruptTriggerEdgePort0 = AC_INTERRUPT_TRIGGER_EDGE_FALL;
						EA = 1;
					}
				}
			}
		}
	} else 
	{
		P0IFG &= ~0x31; // clear interrupt flag
		P0IF = 0;
	}	
	
	IEN1 |= 0x20; // port0 interrupt enable
		
#elif defined(DIMMER_SWITCH_TOUCHKEY)
		
#else
#endif	
}

/*********************************************************************
 * @fn      addDimmerSwitch_isrPort1
 *
 * @brief   Port1 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerSwitch_isrPort1(void)
{		
#if defined(DIMMER_SWITCH)	
	IEN2 &= ~0x10; // port1 interrupt disable
	
	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P1INT_VECTOR    
		//__interrupt void P1_ISR(void) 
		{
			// wakeup
			if (aDimmerSwitchWakeup == ADD_DIMMERSWITCH_WAKEUP_KEYDOWN_10S)
			{
				osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
			} else if (aDimmerSwitchWakeup == ADD_DIMMERSWITCH_WAKEUP_KEYDOWN_5S)
			{
				addDimmerSwitch_clearKeyData();
				acDoubleClickCounter_whenKeyUp = 0;
				aDimmerSwitchWakeup = ADD_DIMMERSWITCH_WAKEUP_KEYNORMAL;
				// no operation
			}

			else // P1_6(key cold)
			
			if ((P1IFG & 0x40) == 0x40) 
			{
				if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_DIMMERSWITCH_KEY_COLD == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_DIMMERSWITCH_KEY_COLD == 0))
					{
						aDimmerSwitchKeyCold = addCommon_handleKeyDown();
						P1IFG &= ~0x40; // clear interrupt flag
						P1IF = 0;
						EA = 0;
						PICTL &= ~0x06;
						acInterruptTriggerEdgePort1 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_DIMMERSWITCH_KEY_COLD == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_DIMMERSWITCH_KEY_COLD == 1))
					{
						aDimmerSwitchKeyCold = addCommon_handleKeyUp();
						if (aDimmerSwitchKeyCold == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aDimmerSwitchKeyCold = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aDimmerSwitchKeyCold = AC_KEY_UP_600MS_TO_1S;
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} else 
						{
							osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
						}						
						P1IFG &= ~0x40; // clear interrupt flag
						P1IF = 0;
						EA = 0;
						PICTL |= 0x06;
						acInterruptTriggerEdgePort1 = AC_INTERRUPT_TRIGGER_EDGE_FALL;
						EA = 1;
					}
				}
			}

			else // P1_7(key warm)
			
			if ((P1IFG & 0x80) == 0x80) 
			{
				if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_DIMMERSWITCH_KEY_WARM == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_DIMMERSWITCH_KEY_WARM == 0))
					{
						aDimmerSwitchKeyWarm = addCommon_handleKeyDown();
						P1IFG &= ~0x80; // clear interrupt flag
						P1IF = 0;
						EA = 0;
						PICTL &= ~0x06;
						acInterruptTriggerEdgePort1 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_DIMMERSWITCH_KEY_WARM == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_DIMMERSWITCH_KEY_WARM == 1))
					{
						aDimmerSwitchKeyWarm = addCommon_handleKeyUp();
						if (aDimmerSwitchKeyWarm == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aDimmerSwitchKeyWarm = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aDimmerSwitchKeyWarm = AC_KEY_UP_600MS_TO_1S;
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} else 
						{
							osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
						}						
						P1IFG &= ~0x80; // clear interrupt flag
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
		P1IFG &= ~0xc0; // clear interrupt flag
		P1IF = 0;
	}
	
	IEN2 |= 0x10; // port1 interrupt enable
		
#elif defined(DIMMER_SWITCH_TOUCHKEY)
		
#else
#endif
}

/*********************************************************************
 * @fn      addDimmerSwitch_isrPort2
 *
 * @brief   Port2 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerSwitch_isrPort2(void)
{		
#if defined(DIMMER_SWITCH)	
	IEN2 &= ~0x02; // port2 interrupt disable
	
	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P2INT_VECTOR    
		//__interrupt void P2_ISR(void) 
		{
			// P2_0(key dimming down)
			if ((P2IFG & 0x01) == 0x01)
			{	
				if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_DIMMERSWITCH_KEY_DIMMING_DOWN == 0))
				{	
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_DIMMERSWITCH_KEY_DIMMING_DOWN == 0))
					{
						aDimmerSwitchKeyDimmingDown = addCommon_handleKeyDown();
						P2IFG &= ~0x01; // clear interrupt flag
						P2IF = 0;
						EA = 0;
						PICTL &= ~0x08;
						acInterruptTriggerEdgePort2 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_DIMMERSWITCH_KEY_DIMMING_DOWN == 1))
				{	
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_DIMMERSWITCH_KEY_DIMMING_DOWN == 1))
					{
						aDimmerSwitchKeyDimmingDown = addCommon_handleKeyUp();
						if (aDimmerSwitchKeyDimmingDown == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aDimmerSwitchKeyDimmingDown = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aDimmerSwitchKeyDimmingDown = AC_KEY_UP_600MS_TO_1S;
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} else 
						{
							osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
						}
						P2IFG &= ~0x01; // clear interrupt flag
						P2IF = 0;
						EA = 0;
						PICTL |= 0x08;
						acInterruptTriggerEdgePort2 = AC_INTERRUPT_TRIGGER_EDGE_FALL;
						EA = 1;
					}
				}
			}
		}
	} else 
	{
		P2IFG &= ~0x01; // clear interrupt flag
		P2IF = 0;
	}
	
	IEN2 |= 0x02; // port2 interrupt enable	
	
#elif defined(DIMMER_SWITCH_TOUCHKEY)	
	IEN2 &= ~0x02; // port2 interrupt disable
	
	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P2INT_VECTOR    
		//__interrupt void P2_ISR(void) 
		{
			if ((P2IFG & 0x01) == 0x01) // P2_0 interrupt
			{
				if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_DIMMERSWITCH_APT8L08_TP_INT == 0))
				{	
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_DIMMERSWITCH_APT8L08_TP_INT == 0))
					{
						aDimmerSwitchApt8l08_16bitKeyMap_saved = 0;
						aDimmerSwitchApt8l08_16bitKeyMap = 
							addDimmerSwitch_APT8L08_I2C_readByte(ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_KVR1) * 256 
							+ addDimmerSwitch_APT8L08_I2C_readByte(ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_KVR0);

						if (aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY0)
						{
							aDimmerSwitchKeyOn = addCommon_handleKeyDown();	
						} else if (aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY1)
						{
							aDimmerSwitchKeyOff = addCommon_handleKeyDown();
						} else if (aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY2)
						{
							aDimmerSwitchKeyDimmingUp = addCommon_handleKeyDown();
						} else if (aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY3)
						{
							aDimmerSwitchKeyDimmingDown = addCommon_handleKeyDown();
						} else if (aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY4)
						{
							aDimmerSwitchKeyCold = addCommon_handleKeyDown();
						} else if (aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY5)
						{
							aDimmerSwitchKeyWarm = addCommon_handleKeyDown();
						}
						
						P2IFG &= ~0x01; // clear interrupt flag
						P2IF = 0;
						EA = 0;
						PICTL &= ~0x08;
						acInterruptTriggerEdgePort2 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_DIMMERSWITCH_APT8L08_TP_INT == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_DIMMERSWITCH_APT8L08_TP_INT == 1))
					{
						aDimmerSwitchApt8l08_16bitKeyMap_saved = aDimmerSwitchApt8l08_16bitKeyMap;
						aDimmerSwitchApt8l08_16bitKeyMap = 
							addDimmerSwitch_APT8L08_I2C_readByte(ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_KVR1) * 256 
							+ addDimmerSwitch_APT8L08_I2C_readByte(ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_KVR0);
						
						osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_KEY_TIMING);

						// factory reset
						if (aDimmerSwitchFactoryReset == ADD_DIMMERSWITCH_FACTORYRESET_KEYDOWN_10S)
						{
							osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
						} else if (aDimmerSwitchFactoryReset == ADD_DIMMERSWITCH_FACTORYRESET_KEYDOWN_5S)
						{
							addDimmerSwitch_clearKeyData();
							acDoubleClickCounter_whenKeyUp = 0;
							aDimmerSwitchFactoryReset = ADD_DIMMERSWITCH_FACTORYRESET_KEYNORMAL;
							// no operation
						}

						else // wakeup

						if (aDimmerSwitchWakeup == ADD_DIMMERSWITCH_WAKEUP_KEYDOWN_10S)
						{
							osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
						} else if (aDimmerSwitchWakeup == ADD_DIMMERSWITCH_WAKEUP_KEYDOWN_5S)
						{
							addDimmerSwitch_clearKeyData();
							acDoubleClickCounter_whenKeyUp = 0;
							aDimmerSwitchWakeup = ADD_DIMMERSWITCH_WAKEUP_KEYNORMAL;
							// no operation
						}

						else // key on

						if ((aDimmerSwitchApt8l08_16bitKeyMap_saved & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY0)
							&& (!(aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY0)))
						{
							aDimmerSwitchKeyOn = addCommon_handleKeyUp();
							if (aDimmerSwitchKeyOn == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aDimmerSwitchKeyOn = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aDimmerSwitchKeyOn = AC_KEY_UP_600MS_TO_1S;
									osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
								}
							} else 
							{
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} 

						else // key off

						if ((aDimmerSwitchApt8l08_16bitKeyMap_saved & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY1)
							&& (!(aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY1)))
						{
							aDimmerSwitchKeyOff = addCommon_handleKeyUp();
							if (aDimmerSwitchKeyOff == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aDimmerSwitchKeyOff = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aDimmerSwitchKeyOff = AC_KEY_UP_600MS_TO_1S;
									osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
								}
							} else 
							{
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} 

						else // key dimming up

						if ((aDimmerSwitchApt8l08_16bitKeyMap_saved & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY2)
							&& (!(aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY2)))
						{
							aDimmerSwitchKeyDimmingUp = addCommon_handleKeyUp();
							if (aDimmerSwitchKeyDimmingUp == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aDimmerSwitchKeyDimmingUp = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aDimmerSwitchKeyDimmingUp = AC_KEY_UP_600MS_TO_1S;
									osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
								}
							} else 
							{
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} 

						else // key dimming down

						if ((aDimmerSwitchApt8l08_16bitKeyMap_saved & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY3)
							&& (!(aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY3)))
						{
							aDimmerSwitchKeyDimmingDown = addCommon_handleKeyUp();
							if (aDimmerSwitchKeyDimmingDown == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aDimmerSwitchKeyDimmingDown = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aDimmerSwitchKeyDimmingDown = AC_KEY_UP_600MS_TO_1S;
									osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
								}
							} else 
							{
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} 

						else // key cold

						if ((aDimmerSwitchApt8l08_16bitKeyMap_saved & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY4)
							&& (!(aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY4)))
						{
							aDimmerSwitchKeyCold = addCommon_handleKeyUp();
							if (aDimmerSwitchKeyCold == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aDimmerSwitchKeyCold = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aDimmerSwitchKeyCold = AC_KEY_UP_600MS_TO_1S;
									osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
								}
							} else 
							{
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} 

						else // key warm

						if ((aDimmerSwitchApt8l08_16bitKeyMap_saved & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY5)
							&& (!(aDimmerSwitchApt8l08_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY5)))
						{
							aDimmerSwitchKeyWarm = addCommon_handleKeyUp();
							if (aDimmerSwitchKeyWarm == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aDimmerSwitchKeyWarm = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aDimmerSwitchKeyWarm = AC_KEY_UP_600MS_TO_1S;
									osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
								}
							} else 
							{
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						}
						
						P2IFG &= ~0x01; // clear interrupt flag
						P2IF = 0;
						EA = 0;
						PICTL |= 0x08;
						acInterruptTriggerEdgePort2 = AC_INTERRUPT_TRIGGER_EDGE_FALL;
						EA = 1;
					}
				}
			}
		}
	} else 
	{
		P2IFG &= ~0x01; // clear interrupt flag
		P2IF = 0;
	}
	
	IEN2 |= 0x02; // port2 interrupt enable		

#else
#endif
}

/*********************************************************************
 * @fn      addDimmerSwitch_sendDestinationAddressConfigure
 *
 * @brief   Destination address configure.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_sendDestinationAddressConfigure(uint8 endPoint)
{
	zclDimmerSwitch_DstAddr.addrMode = (afAddrMode_t)AddrGroup;
	zclDimmerSwitch_DstAddr.endPoint = endPoint;
	zclDimmerSwitch_DstAddr.addr.shortAddr = aDimmerSwitchEndpoint_GroupID;
}

/*********************************************************************
 * @fn		addDimmerSwitch_zclLighting_ColorControl_Send_StepColorTemperatureCmd_Up
 *
 * @brief	Call to send out a Step Color Temperature Command
 *
 * @param	none
 *
 * @return	none
 */
static void addDimmerSwitch_zclLighting_ColorControl_Send_StepColorTemperatureCmd_Up(uint8 endPoint)
{
	uint8 buf[4];

	if ((colorTemperature + stepSize) > 367)
	{
		colorTemperature = 367;
	} else 
	{
		colorTemperature += stepSize;	
	}
	buf[0] = LO_UINT16(colorTemperature);
	buf[1] = HI_UINT16(colorTemperature);
	buf[2] = LO_UINT16(transTime);
	buf[3] = HI_UINT16(transTime);

	zcl_SendCommand( endPoint, &zclDimmerSwitch_DstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
							COMMAND_LIGHTING_MOVE_TO_COLOR_TEMPERATURE, TRUE,
							ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0, 0, 4, buf );
}

/*********************************************************************
 * @fn		addDimmerSwitch_zclLighting_ColorControl_Send_StepColorTemperatureCmd_Down
 *
 * @brief	Call to send out a Step Color Temperature Command
 *
 * @param	none
 *
 * @return	none
 */
static void addDimmerSwitch_zclLighting_ColorControl_Send_StepColorTemperatureCmd_Down(uint8 endPoint)
{
	uint8 buf[4];

	if ((colorTemperature - stepSize) < 150)
	{
		colorTemperature = 150;
	} else 
	{
		colorTemperature -= stepSize;
	}
	buf[0] = LO_UINT16(colorTemperature);
	buf[1] = HI_UINT16(colorTemperature);
	buf[2] = LO_UINT16(transTime);
	buf[3] = HI_UINT16(transTime);

	zcl_SendCommand( endPoint, &zclDimmerSwitch_DstAddr, ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
							COMMAND_LIGHTING_MOVE_TO_COLOR_TEMPERATURE, TRUE,
							ZCL_FRAME_CLIENT_SERVER_DIR, FALSE, 0, 0, 4, buf );
}

/*********************************************************************
 * @fn      addDimmerSwitch_sendLevelControlMove
 *
 * @brief   Dimmer switch.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerSwitch_sendLevelControlMove(void)
{	
	if (aDimmerSwitchKeyDimmingUp == AC_KEY_DOWN)
	{
		addDimmerSwitch_sendDestinationAddressConfigure(DIMMERSWITCH_ENDPOINT);
		zclGeneral_SendLevelControlMove( DIMMERSWITCH_ENDPOINT, &zclDimmerSwitch_DstAddr, LEVEL_MOVE_UP, 45, FALSE, 0 );
	} else if (aDimmerSwitchKeyDimmingDown == AC_KEY_DOWN)
	{
		addDimmerSwitch_sendDestinationAddressConfigure(DIMMERSWITCH_ENDPOINT);
		zclGeneral_SendLevelControlMove( DIMMERSWITCH_ENDPOINT, &zclDimmerSwitch_DstAddr, LEVEL_MOVE_DOWN, 45, FALSE, 0 );
	}	
}

/*********************************************************************
 * @fn      addDimmerSwitch_clearKeyData
 *
 * @brief   Dimmer switch.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_clearKeyData(void)
{
	aDimmerSwitchKeyOn = AC_KEY_NORMAL;
	aDimmerSwitchKeyOff = AC_KEY_NORMAL;		
	aDimmerSwitchKeyDimmingUp = AC_KEY_NORMAL;		
	aDimmerSwitchKeyDimmingDown = AC_KEY_NORMAL;		
	aDimmerSwitchKeyCold = AC_KEY_NORMAL;		
	aDimmerSwitchKeyWarm = AC_KEY_NORMAL;
}

/*********************************************************************
 * @fn      addDimmerSwitch_factoryResetCheck
 *
 * @brief   Dimmer switch.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerSwitch_factoryResetCheck(void)
{	
	uint16 temp_16bitKeyMap = 0;

	temp_16bitKeyMap = 
		addDimmerSwitch_APT8L08_I2C_readByte(ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_KVR1) * 256 
		+ addDimmerSwitch_APT8L08_I2C_readByte(ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_KVR0);
	
	if (acDeviceKeyTimingCounter == 50) // 5000ms
	{
		if ((temp_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY0)
			&& (temp_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY1))
		{
			aDimmerSwitchFactoryReset = ADD_DIMMERSWITCH_FACTORYRESET_KEYDOWN_5S;
		}
	} else if (acDeviceKeyTimingCounter == 100) // 10000ms
	{
		if ((aDimmerSwitchFactoryReset == ADD_DIMMERSWITCH_FACTORYRESET_KEYDOWN_5S)
			&& (temp_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY0)
			&& (temp_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY1))
		{
			aDimmerSwitchFactoryReset = ADD_DIMMERSWITCH_FACTORYRESET_KEYDOWN_10S;
		}
	}
}

/*********************************************************************
 * @fn      addDimmerSwitch_wakeupCheck
 *
 * @brief   Dimmer switch.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerSwitch_wakeupCheck(void)
{	
	uint16 temp_16bitKeyMap = 0;

	temp_16bitKeyMap = 
		addDimmerSwitch_APT8L08_I2C_readByte(ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_KVR1) * 256 
		+ addDimmerSwitch_APT8L08_I2C_readByte(ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_KVR0);
	
	if (acDeviceKeyTimingCounter == 50) // 5000ms
	{
		if ((temp_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY2)
			&& (temp_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY3))
		{
			aDimmerSwitchWakeup = ADD_DIMMERSWITCH_WAKEUP_KEYDOWN_5S;
		}
	} else if (acDeviceKeyTimingCounter == 100) // 10000ms
	{
		if ((aDimmerSwitchWakeup == ADD_DIMMERSWITCH_WAKEUP_KEYDOWN_5S)
			&& (temp_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY2)
			&& (temp_16bitKeyMap & ADD_DIMMERSWITCH_APT8L08_TOUCHKEY3))
		{
			aDimmerSwitchWakeup = ADD_DIMMERSWITCH_WAKEUP_KEYDOWN_10S;
		}
	}
}

/*********************************************************************
 * @fn      addDimmerSwitch_setGroupID
 *
 * @brief   Dimmer switch.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerSwitch_setGroupID(uint8 endPoint, aps_Group_t *group)
{
	if (endPoint == DIMMERSWITCH_ENDPOINT)
	{
		aDimmerSwitchEndpoint_GroupID = group->ID;
		osal_nv_write(ADD_DIMMERSWITCH_NV_ITEM_ID_ENDPOINT_GROUPID, 0, 2, &aDimmerSwitchEndpoint_GroupID);
	}
}

#if defined(DIMMER_SWITCH)	
#elif defined(DIMMER_SWITCH_TOUCHKEY)

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_init
 *
 * @brief   APT8L08 initial.
 *
 * @param   none
 *
 * @return  none
 */
void addDimmerSwitch_APT8L08_init(void)
{	
	addDimmerSwitch_APT8L08_initHal();
	addCommon_delayMS(500);
	addDimmerSwitch_APT8L08_initAl();
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_initHal
 *
 * @brief   APT8L08 initial hardware abstract layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_APT8L08_initHal(void)
{
	APCFG &= ~0x30; // SDA on P0_4, SCL on P0_5
	P0SEL &= ~0x30; 
	P0DIR |= 0x30;
	P2INP &= ~0x20; // port0 all pull up
	P0INP |= 0x30;
	
	P2SEL &= ~0x01; // TP_INT on port P2_0
	P2DIR &= ~0x01;
	P2INP &= ~0x80; // port2 all pull up
	P2INP &= ~0x01;

	EA = 0;
	
	MicroWait(50000);
	MicroWait(50000);

	P2IEN |= 0x01; // P2_0 interrupt enable
	PICTL |= 0x08; // fall edge
	P2IFG &= ~0x01; // clear interrupt flag
	IEN2 |= 0x02; // port2 interrupt enable
	
	IP0 |= (1 << 1); // set port2 interrupt highest priority
	IP1 |= (1 << 1);
	
	EA = 1;

	ADD_DIMMERSWITCH_APT8L08_SDA = 1;
	ADD_DIMMERSWITCH_APT8L08_SCL = 1;
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_initAl
 *
 * @brief   APT8L08 initial application layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_APT8L08_initAl(void)
{
	addCommon_delayMS(ADD_DIMMERSWITCH_APT8L08_POWERON_DELAY);

	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_SYSCON, ADD_DIMMERSWITCH_APT8L08_MODE_CONFIGURE);
	addCommon_delayMS(10);
	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_SYSCON, ADD_DIMMERSWITCH_APT8L08_MODE_NORMAL);
	addCommon_delayMS(10);
	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_SYSCON, ADD_DIMMERSWITCH_APT8L08_MODE_CONFIGURE);
	addCommon_delayMS(10);
	
	for (uint8 i = 0; i < 16; i++)
	{
		addDimmerSwitch_APT8L08_I2C_writeRegister(
			aDimmerSwitchApt8l08_registerAddress[i], aDimmerSwitchApt8l08_registerData[i]);
	}
	
	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_GSR, ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_GSR); //gsr
	addCommon_delayMS(10);
	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_MCON, ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_MCON); //mcon 当使用AT8L08时,高4位一定要配置为“5";当使用AT8L16时,高4位配置为“0”
	addCommon_delayMS(10);
	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_K0_ENB, ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_K0_ENB); //K0_CON
	addCommon_delayMS(10);
	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_K1_ENB, ADD_DIMMERSWITCH_APT8L08_REGISTER_DATA_K1_ENB); //K1_CON
	addCommon_delayMS(10);

	//addDimmerSwitch_APT8L08_I2C_writeRegister(
	//		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_KOR00, aDimmerSwitchApt8l08_tkFin8bit[0]);
	//	addCommon_delayMS(10);
	for (uint8 i = 0; i < 16; i++)
	{
		addDimmerSwitch_APT8L08_I2C_writeRegister(i, aDimmerSwitchApt8l08_tkFin8bit[i]);
		addCommon_delayMS(10);
	}

	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_FILTER0, 0xac); //FILTER<1:0>, 分别对应原始值0 1 2 3 阶滤波；FILTER<3:2>，分别对应参考值寄存器更新时间2 4 8 16 次有效后更新； FILTER<5:4>, 分别对应参考值0 1 2 3 阶滤波ILTER<6>按键键值更新不设高上限
	addCommon_delayMS(10);
	//addDimmerSwitch_APT8L08_I2C_writeRegister(
	//	ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_SENSE_CON, 0x40);	//加快扫描速度
	//addCommon_delayMS(10);
	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_REF_UTH, 0x60); //参考寄存器更新阈值 
	addCommon_delayMS(10);
	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_KEY_ATH, 0xff); //开发模式寄存器 直接触摸PAD设置“0XFF",默认“0X10”
	addCommon_delayMS(10);
	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_MCONH, 0x42); //BIT 6 置“1” 去掉longpress 16s
	addCommon_delayMS(10);
	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_FILTER1, 0x00); //开发模式寄存器 直接触摸PAD设置“0X08",默认“0X00”
	addCommon_delayMS(10);
	addDimmerSwitch_APT8L08_I2C_writeRegister(
		ADD_DIMMERSWITCH_APT8L08_REGISTER_ADDRESS_SYSCON, ADD_DIMMERSWITCH_APT8L08_MODE_NORMAL);
	addCommon_delayMS(10);

	addCommon_delayMS(500);	
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_I2C_start
 *
 * @brief   APT8L08 i2c start.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_APT8L08_I2C_start(void)
{	
	ADD_DIMMERSWITCH_APT8L08_SDA = 1;
	MicroWait(5);
	ADD_DIMMERSWITCH_APT8L08_SCL = 1;
	MicroWait(5);
	ADD_DIMMERSWITCH_APT8L08_SDA = 0;
	MicroWait(5);	
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_I2C_stop
 *
 * @brief   APT8L08 i2c stop.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_APT8L08_I2C_stop(void)
{	
	ADD_DIMMERSWITCH_APT8L08_SDA = 0;
	MicroWait(5);
	ADD_DIMMERSWITCH_APT8L08_SCL = 1;
	MicroWait(5);
	ADD_DIMMERSWITCH_APT8L08_SDA = 1;
	MicroWait(5);	
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_I2C_waitACK
 *
 * @brief   APT8L08 i2c wait ACK.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_APT8L08_I2C_waitACK(void)
{
	uint8 bTemp;

	ADD_DIMMERSWITCH_APT8L08_SCL = 1;
	ADD_DIMMERSWITCH_APT8L08_SDA_DIR(ADD_DIMMERSWITCH_INPUT);
	for (bTemp = 0; bTemp <= 250; bTemp++)
	{
		if (ADD_DIMMERSWITCH_APT8L08_SDA)
		{
			// no operation
		} else 
		{
			break;
		}
	}

	if (bTemp >= 250)
	{
		aDimmerSwitchApt8l08_i2cWaitStatus = 0x0a;
	}
	ADD_DIMMERSWITCH_APT8L08_SCL = 0;
	ADD_DIMMERSWITCH_APT8L08_SDA_DIR(ADD_DIMMERSWITCH_OUTPUT);
	MicroWait(5);
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_I2C_noACK
 *
 * @brief   APT8L08 i2c no ACK.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_APT8L08_I2C_noACK(void)
{
	ADD_DIMMERSWITCH_APT8L08_SDA = 1;
	MicroWait(5);
	ADD_DIMMERSWITCH_APT8L08_SCL = 1;
	MicroWait(5);
	ADD_DIMMERSWITCH_APT8L08_SCL = 0;
	MicroWait(5);
	ADD_DIMMERSWITCH_APT8L08_SDA = 0;
	MicroWait(5);
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_I2C_ACK
 *
 * @brief   APT8L08 i2c ACK.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_APT8L08_I2C_ACK(void)
{
	ADD_DIMMERSWITCH_APT8L08_SDA = 0;
	MicroWait(5);
	ADD_DIMMERSWITCH_APT8L08_SCL = 1;
	MicroWait(5);
	ADD_DIMMERSWITCH_APT8L08_SCL = 0;
	MicroWait(5);
	ADD_DIMMERSWITCH_APT8L08_SDA = 1;
	MicroWait(5);
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_I2C_write8bit
 *
 * @brief   APT8L08 i2c write 8 bit.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_APT8L08_I2C_write8bit(uint8 dataWrite)
{
	ADD_DIMMERSWITCH_APT8L08_SCL = 0;
	MicroWait(5);
	for (uint8 i = 0; i < 8; i++)
	{
		if ((dataWrite << i) & 0x80)
			ADD_DIMMERSWITCH_APT8L08_SDA = 1;
		else			
			ADD_DIMMERSWITCH_APT8L08_SDA = 0;
		MicroWait(5);
		ADD_DIMMERSWITCH_APT8L08_SCL = 1;
		MicroWait(5);
		ADD_DIMMERSWITCH_APT8L08_SCL = 0;
		MicroWait(5);		
	}
	ADD_DIMMERSWITCH_APT8L08_SCL = 0;
	MicroWait(5);
	ADD_DIMMERSWITCH_APT8L08_SDA = 1;
	MicroWait(5);	
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_I2C_read8bit
 *
 * @brief   APT8L08 i2c read 8 bit.
 *
 * @param   none
 *
 * @return  none
 */
static uint8 addDimmerSwitch_APT8L08_I2C_read8bit(void)
{	
	uint8 dataReceive;

	ADD_DIMMERSWITCH_APT8L08_SDA_DIR(ADD_DIMMERSWITCH_INPUT);
	for (uint8 i = 0; i < 8; i++)
	{
		dataReceive = dataReceive << 1;
		ADD_DIMMERSWITCH_APT8L08_SCL = 1;
		MicroWait(5);
		if (ADD_DIMMERSWITCH_APT8L08_SDA)
			dataReceive = dataReceive | 0x01;
		ADD_DIMMERSWITCH_APT8L08_SCL = 0;
		MicroWait(5);		
	}
	ADD_DIMMERSWITCH_APT8L08_SDA_DIR(ADD_DIMMERSWITCH_OUTPUT);
	ADD_DIMMERSWITCH_APT8L08_SCL = 0;
	ADD_DIMMERSWITCH_APT8L08_SDA = 1;
	MicroWait(5);

	return dataReceive;
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_I2C_writeByte
 *
 * @brief   APT8L08 i2c write byte.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_APT8L08_I2C_writeByte(uint8 address, uint8 dataWrite)
{
	addDimmerSwitch_APT8L08_I2C_start();

	addDimmerSwitch_APT8L08_I2C_write8bit(0xac);
	addDimmerSwitch_APT8L08_I2C_waitACK();
	addDimmerSwitch_APT8L08_I2C_write8bit(address);
	addDimmerSwitch_APT8L08_I2C_waitACK();

	addDimmerSwitch_APT8L08_I2C_write8bit(dataWrite);
	addDimmerSwitch_APT8L08_I2C_waitACK();

	addDimmerSwitch_APT8L08_I2C_stop();
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_I2C_readByte
 *
 * @brief   APT8L08 i2c read byte.
 *
 * @param   none
 *
 * @return  none
 */
static uint8 addDimmerSwitch_APT8L08_I2C_readByte(uint8 address)
{
	uint8 dataReceive;
	
	addDimmerSwitch_APT8L08_I2C_start();

	addDimmerSwitch_APT8L08_I2C_write8bit(0xac);
	addDimmerSwitch_APT8L08_I2C_waitACK();
	addDimmerSwitch_APT8L08_I2C_write8bit(address);
	addDimmerSwitch_APT8L08_I2C_waitACK();

	addDimmerSwitch_APT8L08_I2C_start();

	addDimmerSwitch_APT8L08_I2C_write8bit(0xad);
	addDimmerSwitch_APT8L08_I2C_waitACK();

	dataReceive = addDimmerSwitch_APT8L08_I2C_read8bit();
	addDimmerSwitch_APT8L08_I2C_noACK();
	
	addDimmerSwitch_APT8L08_I2C_stop();

	return dataReceive;
}

/*********************************************************************
 * @fn      addDimmerSwitch_APT8L08_I2C_writeRegister
 *
 * @brief   APT8L08 i2c write register.
 *
 * @param   none
 *
 * @return  none
 */
static void addDimmerSwitch_APT8L08_I2C_writeRegister(uint8 address, uint8 dataWrite)
{
	do
	{
		addDimmerSwitch_APT8L08_I2C_writeByte(address, dataWrite);
	} while (dataWrite != addDimmerSwitch_APT8L08_I2C_readByte(address));
}
	
#else
#endif

