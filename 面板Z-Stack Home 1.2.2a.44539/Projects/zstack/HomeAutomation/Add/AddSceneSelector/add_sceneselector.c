/*********************************************************************
 * INCLUDES
 */
#include "add_sceneselector.h"

/*********************************************************************
 * LOCAL VARIABLES
 */
volatile byte aSceneSelectorKeyScene1 = AC_KEY_NORMAL;
volatile byte aSceneSelectorKeyScene2 = AC_KEY_NORMAL;
volatile byte aSceneSelectorKeyScene3 = AC_KEY_NORMAL;
volatile byte aSceneSelectorKeyScene4 = AC_KEY_NORMAL;
volatile byte aSceneSelectorKeyDimmingUp = AC_KEY_NORMAL;
volatile byte aSceneSelectorKeyDimmingDown = AC_KEY_NORMAL;

uint16 aSceneSelectorEndpoint1_SceneID = 1;
uint16 aSceneSelectorEndpoint2_SceneID = 2;
uint16 aSceneSelectorEndpoint3_SceneID = 3;
uint16 aSceneSelectorEndpoint4_SceneID = 4;

uint16 aSceneSelectorEndpoint1_GroupID = 1;
uint16 aSceneSelectorEndpoint2_GroupID = 2;
uint16 aSceneSelectorEndpoint3_GroupID = 3;
uint16 aSceneSelectorEndpoint4_GroupID = 4;

volatile uint8 aSceneSelectorCurrentEndpoint = SCENESELECTOR_ENDPOINT1;

volatile uint8 aSceneSelectorFactoryReset = ADD_SCENESELECTOR_FACTORYRESET_KEYNORMAL;
volatile uint8 aSceneSelectorWakeup = ADD_SCENESELECTOR_WAKEUP_KEYNORMAL;

#if defined(SCENE_SELECTOR)
#elif defined(SCENE_SELECTOR_TOUCHKEY)
uint8 aSceneSelectorApt8l08_i2cWaitStatus;

const uint8 aSceneSelectorApt8l08_registerAddress[16] = {
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_GSR,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_MCON,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_FILTER0,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_K0_ENB,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_K1_ENB,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_SENSE_CON,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_SENSE_S,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_GSR_K07,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_GSR_K17,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_REF_UTH,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_KEY_ATH,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_DSMIT,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_MCONH,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_FILTER1,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_K0_CON,
	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_K1_CON
};

const uint8 aSceneSelectorApt8l08_registerData[16] = {
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_GSR,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_MCON,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_FILTER0,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_K0_ENB,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_K1_ENB,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_SENSE_CON,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_SENSE_S,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_GSR_K07,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_GSR_K17,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_REF_UTH,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_KEY_ATH,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_DSMIT,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_MCONH,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_FILTER1,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_K0_CON,
	ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_K1_CON
};

const uint8 aSceneSelectorApt8l08_tkFin8bit[16] 
	= {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
volatile uint16 aSceneSelectorApt8l08_16bitKeyMap = 0;
volatile uint16 aSceneSelectorApt8l08_16bitKeyMap_saved = 0;

#else
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void addSceneSelector_init(void);
static void addSceneSelector_initHal(void);
static void addSceneSelector_initAl(void);
static void addSceneSelector_initNvItemID(void);
static void addSceneSelector_initNvData(void);
uint16 addSceneSelector_eventLoop(uint8 task_id, uint16 events);
static void addSceneSelector_polling(void);
static void addSceneSelector_handleOneTime(void);
void addSceneSelector_isrPort0(void);
void addSceneSelector_isrPort1(void);
void addSceneSelector_isrPort2(void);

static void addSceneSelector_sendDestinationAddressConfigure(uint8 endPoint);
void addSceneSelector_sendLevelControlMove(void);
static void addSceneSelector_clearKeyData(void);
void addSceneSelector_factoryResetCheck(void);
void addSceneSelector_wakeupCheck(void);
void addSceneSelector_setGroupID(uint8 endPoint, aps_Group_t *group);
void addSceneSelector_setSceneID(uint8 endPoint, zclGeneral_Scene_t *scene);

#if defined(SCENE_SELECTOR)	
#elif defined(SCENE_SELECTOR_TOUCHKEY)
void addSceneSelector_APT8L08_init(void);
static void addSceneSelector_APT8L08_initHal(void);
static void addSceneSelector_APT8L08_initAl(void);
static void addSceneSelector_APT8L08_I2C_start(void);
static void addSceneSelector_APT8L08_I2C_stop(void);
static void addSceneSelector_APT8L08_I2C_waitACK(void);
static void addSceneSelector_APT8L08_I2C_noACK(void);
static void addSceneSelector_APT8L08_I2C_ACK(void);
static void addSceneSelector_APT8L08_I2C_write8bit(uint8 dataWrite);
static uint8 addSceneSelector_APT8L08_I2C_read8bit(void);
static void addSceneSelector_APT8L08_I2C_writeByte(uint8 address, uint8 dataWrite);
static uint8 addSceneSelector_APT8L08_I2C_readByte(uint8 address);
static void addSceneSelector_APT8L08_I2C_writeRegister(uint8 address, uint8 dataWrite);
	
#else
#endif

/*********************************************************************
 * @fn      addSceneSelector_init
 *
 * @brief   Scene selector initial.
 *
 * @param   none
 *
 * @return  none
 */
void addSceneSelector_init(void)
{	
	addCommon_init();
	
#if defined(SCENE_SELECTOR)
	addSceneSelector_initHal();
	addCommon_delayMS(500);
	addSceneSelector_initAl();
	
#elif defined(SCENE_SELECTOR_TOUCHKEY)
	addSceneSelector_APT8L08_init();
	
#else
#endif

	acPointer_NwkState = &zclSceneSelector_NwkState;
	acPointer_TaskID = &zclSceneSelector_TaskID;

	addCommon_wakeup();
}

/*********************************************************************
 * @fn      addSceneSelector_initHal
 *
 * @brief   Scene selector initial hardware abstract layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_initHal(void)
{
	P2SEL &= ~0x01; // P2_0(key scene4)
	P2DIR &= ~0x01;
	P2INP &= ~0x80; // port2 all pull up
	P2INP &= ~0x01;
	
	APCFG &= ~0x31; // P0_4(key scene1) P0_5(key scene2) P0_0(key scene3)
	P0SEL &= ~0x31;
	P0DIR &= ~0x31;
	P2INP &= ~0x20; // port0 all pull up
	P0INP &= ~0x31;
	
	P1SEL &= ~0xc0; // P1_6(key dimming up) P1_7(key dimming down)
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
 * @fn      addSceneSelector_initAl
 *
 * @brief   Scene selector initial application layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_initAl(void)
{	
	addSceneSelector_initNvItemID();
	addSceneSelector_initNvData();
}

/*********************************************************************
 * @fn      addSceneSelector_initNvItemID
 *
 * @brief   Scene selector initial non-volatile flash item ID.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_initNvItemID(void)
{
	osal_nv_item_init(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT1_SCENEID, 2, &aSceneSelectorEndpoint1_SceneID);
	osal_nv_item_init(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT2_SCENEID, 2, &aSceneSelectorEndpoint2_SceneID);
	osal_nv_item_init(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT3_SCENEID, 2, &aSceneSelectorEndpoint3_SceneID);
	osal_nv_item_init(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT4_SCENEID, 2, &aSceneSelectorEndpoint4_SceneID);

	osal_nv_item_init(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT1_GROUPID, 2, &aSceneSelectorEndpoint1_GroupID);
	osal_nv_item_init(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT2_GROUPID, 2, &aSceneSelectorEndpoint2_GroupID);
	osal_nv_item_init(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT3_GROUPID, 2, &aSceneSelectorEndpoint3_GroupID);
	osal_nv_item_init(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT4_GROUPID, 2, &aSceneSelectorEndpoint4_GroupID);
}

/*********************************************************************
 * @fn      addSceneSelector_initNvData
 *
 * @brief   Scene selector initial non-volatile flash data.
 *
 * @param   none
 *
 * @return  none
 */
 static void addSceneSelector_initNvData(void)
{
	osal_nv_read(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT1_SCENEID, 0, 2, &aSceneSelectorEndpoint1_SceneID);
	osal_nv_read(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT2_SCENEID, 0, 2, &aSceneSelectorEndpoint2_SceneID);
	osal_nv_read(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT3_SCENEID, 0, 2, &aSceneSelectorEndpoint3_SceneID);
	osal_nv_read(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT4_SCENEID, 0, 2, &aSceneSelectorEndpoint4_SceneID);
	
	osal_nv_read(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT1_GROUPID, 0, 2, &aSceneSelectorEndpoint1_GroupID);
	osal_nv_read(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT2_GROUPID, 0, 2, &aSceneSelectorEndpoint2_GroupID);
	osal_nv_read(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT3_GROUPID, 0, 2, &aSceneSelectorEndpoint3_GroupID);
	osal_nv_read(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT4_GROUPID, 0, 2, &aSceneSelectorEndpoint4_GroupID);
}

/*********************************************************************
 * @fn      addSceneSelector_eventLoop
 *
 * @brief   Scene selector Event Loop Processor for zclGeneral.
 *
 * @param   uint8 task_id, uint16 events
 *
 * @return  uint16
 */
uint16 addSceneSelector_eventLoop(uint8 task_id, uint16 events)
{
	if (events & AC_EVENT_POLLING)//5S进一次事件
	{
		addSceneSelector_polling();
		
		if (acDelayJoinNetwork != 0)//两分钟的控制
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
		//add by jc按键按下，唤醒唤醒1秒，发送2次data request
		addCommon_wakeup();
		acDelayJoinNetwork = 1;
		osal_start_timerEx(*acPointer_TaskID, AC_EVENT_POLLING, 1000);
		//end of add by jc
		addSceneSelector_handleOneTime();
		return ( events ^ AC_EVENT_HANDLE_ONE_TIME);
	}
	
	if (events & AC_EVENT_DOUBLE_CLICK_CHECK)
	{
		//add by jc按键按下，唤醒唤醒5秒，发送2次data request
		addCommon_wakeup();
		acDelayJoinNetwork = 1;
		osal_start_timerEx(*acPointer_TaskID, AC_EVENT_POLLING, 1000);
		//end of add by jc
		if (acDoubleClickCounter_whenKeyDown >= 2)
		{
			acDoubleClickCounter_whenKeyUp = acDoubleClickCounter_whenKeyDown;
		}
		acDoubleClickCounter_whenKeyDown = 0;
		addSceneSelector_handleOneTime();
		acDoubleClickCheckFlag = FALSE;
		return ( events ^ AC_EVENT_DOUBLE_CLICK_CHECK);
	}
	
	return addCommon_eventLoop(task_id, events);
}

/*********************************************************************
 * @fn      addSceneSelector_polling
 *
 * @brief   Scene selector.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_polling(void)
{
	addCommon_polling();
}

/*********************************************************************
 * @fn      addSceneSelector_handleOneTime
 *
 * @brief   Scene selector handle one time.
 *
 * @param   none
 *
 * @return  none
 */
void addSceneSelector_handleOneTime(void)
{
	addCommon_handleOneTime();

	// factory reset
	if (aSceneSelectorFactoryReset == ADD_SCENESELECTOR_FACTORYRESET_KEYDOWN_10S)
	{
		addSceneSelector_clearKeyData();	
		acDoubleClickCounter_whenKeyUp = 0;
		aSceneSelectorFactoryReset = ADD_SCENESELECTOR_FACTORYRESET_KEYNORMAL;
		addCommon_factoryReset(); //恢复出厂设置
	} 

	else // wakeup

	if (aSceneSelectorWakeup == ADD_SCENESELECTOR_WAKEUP_KEYDOWN_10S)
	{
		addSceneSelector_clearKeyData();	
		acDoubleClickCounter_whenKeyUp = 0;
		aSceneSelectorWakeup = ADD_SCENESELECTOR_WAKEUP_KEYNORMAL;
		addCommon_wakeup();//唤醒两分钟
	}

	else // key0
	
	if (aSceneSelectorKeyScene1 == AC_KEY_DOWN) // long press
	{
		addSceneSelector_clearKeyData();
		aSceneSelectorKeyScene1 = AC_KEY_DOWN;
	} else if ((aSceneSelectorKeyScene1 == AC_KEY_UP_600MS_TO_1S) || (aSceneSelectorKeyScene1 == AC_KEY_UP_1S_TO_5S))
	{
		addSceneSelector_clearKeyData();//场景调用
		addSceneSelector_sendDestinationAddressConfigure(SCENESELECTOR_ENDPOINT1);
		zclGeneral_SendSceneRecall(SCENESELECTOR_ENDPOINT1, &zclSceneSelector_DstAddr, aSceneSelectorEndpoint1_GroupID, aSceneSelectorEndpoint1_SceneID, FALSE, 0);
		aSceneSelectorCurrentEndpoint = SCENESELECTOR_ENDPOINT1;
	} else if ((aSceneSelectorKeyScene1 == AC_KEY_UP_5S_TO_10S) || (aSceneSelectorKeyScene1 == AC_KEY_UP_ABOVE_10S))
	{
		addSceneSelector_clearKeyData();//场景存储
		addSceneSelector_sendDestinationAddressConfigure(SCENESELECTOR_ENDPOINT1);
		zclGeneral_SendSceneStore(SCENESELECTOR_ENDPOINT1, &zclSceneSelector_DstAddr, aSceneSelectorEndpoint1_GroupID, aSceneSelectorEndpoint1_SceneID, FALSE, 0);
		aSceneSelectorCurrentEndpoint = SCENESELECTOR_ENDPOINT1;
	}
	
	else // key1
	
	if (aSceneSelectorKeyScene2 == AC_KEY_DOWN) // long press
	{
		addSceneSelector_clearKeyData();
		aSceneSelectorKeyScene2 = AC_KEY_DOWN;
	} else if ((aSceneSelectorKeyScene2 == AC_KEY_UP_600MS_TO_1S) || (aSceneSelectorKeyScene2 == AC_KEY_UP_1S_TO_5S))
	{
		addSceneSelector_clearKeyData();
		addSceneSelector_sendDestinationAddressConfigure(SCENESELECTOR_ENDPOINT2);
		zclGeneral_SendSceneRecall(SCENESELECTOR_ENDPOINT2, &zclSceneSelector_DstAddr, aSceneSelectorEndpoint2_GroupID, aSceneSelectorEndpoint2_SceneID, FALSE, 0);
		aSceneSelectorCurrentEndpoint = SCENESELECTOR_ENDPOINT2;
	} else if ((aSceneSelectorKeyScene2 == AC_KEY_UP_5S_TO_10S) || (aSceneSelectorKeyScene2 == AC_KEY_UP_ABOVE_10S))
	{
		addSceneSelector_clearKeyData();
		addSceneSelector_sendDestinationAddressConfigure(SCENESELECTOR_ENDPOINT2);
		zclGeneral_SendSceneStore(SCENESELECTOR_ENDPOINT2, &zclSceneSelector_DstAddr, aSceneSelectorEndpoint2_GroupID, aSceneSelectorEndpoint2_SceneID, FALSE, 0);
		aSceneSelectorCurrentEndpoint = SCENESELECTOR_ENDPOINT2;
	}

	else // key2
	
	if (aSceneSelectorKeyScene3 == AC_KEY_DOWN) // long press
	{
		addSceneSelector_clearKeyData();
		aSceneSelectorKeyScene3 = AC_KEY_DOWN;
	} else if ((aSceneSelectorKeyScene3 == AC_KEY_UP_600MS_TO_1S) || (aSceneSelectorKeyScene3 == AC_KEY_UP_1S_TO_5S))
	{
		addSceneSelector_clearKeyData();
		addSceneSelector_sendDestinationAddressConfigure(SCENESELECTOR_ENDPOINT3);
		zclGeneral_SendSceneRecall(SCENESELECTOR_ENDPOINT3, &zclSceneSelector_DstAddr, aSceneSelectorEndpoint3_GroupID, aSceneSelectorEndpoint3_SceneID, FALSE, 0);
		aSceneSelectorCurrentEndpoint = SCENESELECTOR_ENDPOINT3;
	} else if ((aSceneSelectorKeyScene3 == AC_KEY_UP_5S_TO_10S) || (aSceneSelectorKeyScene3 == AC_KEY_UP_ABOVE_10S))
	{
		addSceneSelector_clearKeyData();
		addSceneSelector_sendDestinationAddressConfigure(SCENESELECTOR_ENDPOINT3);
		zclGeneral_SendSceneStore(SCENESELECTOR_ENDPOINT3, &zclSceneSelector_DstAddr, aSceneSelectorEndpoint3_GroupID, aSceneSelectorEndpoint3_SceneID, FALSE, 0);
		aSceneSelectorCurrentEndpoint = SCENESELECTOR_ENDPOINT3;
	}

	else // key3
	
	if (aSceneSelectorKeyScene4 == AC_KEY_DOWN) // long press
	{
		addSceneSelector_clearKeyData();
		aSceneSelectorKeyScene4 = AC_KEY_DOWN;
	} else if ((aSceneSelectorKeyScene4 == AC_KEY_UP_600MS_TO_1S) || (aSceneSelectorKeyScene4 == AC_KEY_UP_1S_TO_5S))
	{
		addSceneSelector_clearKeyData();
		addSceneSelector_sendDestinationAddressConfigure(SCENESELECTOR_ENDPOINT4);
		zclGeneral_SendSceneRecall(SCENESELECTOR_ENDPOINT4, &zclSceneSelector_DstAddr, aSceneSelectorEndpoint4_GroupID, aSceneSelectorEndpoint4_SceneID, FALSE, 0);
		aSceneSelectorCurrentEndpoint = SCENESELECTOR_ENDPOINT4;
	} else if ((aSceneSelectorKeyScene4 == AC_KEY_UP_5S_TO_10S) || (aSceneSelectorKeyScene4 == AC_KEY_UP_ABOVE_10S))
	{
		addSceneSelector_clearKeyData();
		addSceneSelector_sendDestinationAddressConfigure(SCENESELECTOR_ENDPOINT4);
		zclGeneral_SendSceneStore(SCENESELECTOR_ENDPOINT4, &zclSceneSelector_DstAddr, aSceneSelectorEndpoint4_GroupID, aSceneSelectorEndpoint4_SceneID, FALSE, 0);
		aSceneSelectorCurrentEndpoint = SCENESELECTOR_ENDPOINT4;
	}

	else // keyDimmingUp
	
	if (aSceneSelectorKeyDimmingUp == AC_KEY_DOWN) // long press
	{
		addSceneSelector_clearKeyData();
		aSceneSelectorKeyDimmingUp = AC_KEY_DOWN;
	} else if (aSceneSelectorKeyDimmingUp == AC_KEY_UP_600MS_TO_1S)
	{
		addSceneSelector_clearKeyData();
		if (acDoubleClickCounter_whenKeyUp <= 1) // short press，级调，增亮
		{
			addSceneSelector_sendDestinationAddressConfigure(aSceneSelectorCurrentEndpoint);
			zclGeneral_SendLevelControlStep(aSceneSelectorCurrentEndpoint, &zclSceneSelector_DstAddr, LEVEL_STEP_UP, 35, 1, FALSE, 0 );
		} else // double click，开灯
		{
			addSceneSelector_sendDestinationAddressConfigure(aSceneSelectorCurrentEndpoint);
			zclGeneral_SendOnOff_CmdOn(aSceneSelectorCurrentEndpoint, &zclSceneSelector_DstAddr, FALSE, 0 );
		}
	} else if ((aSceneSelectorKeyDimmingUp == AC_KEY_UP_1S_TO_5S) || (aSceneSelectorKeyDimmingUp == AC_KEY_UP_5S_TO_10S) || (aSceneSelectorKeyDimmingUp == AC_KEY_UP_ABOVE_10S))
	{
		addSceneSelector_clearKeyData();//处理长按抬起后，停止渐变move调光
		addSceneSelector_sendDestinationAddressConfigure(aSceneSelectorCurrentEndpoint);
		zclGeneral_SendLevelControlStop(aSceneSelectorCurrentEndpoint, &zclSceneSelector_DstAddr, FALSE, 0 );
	}

	else // keyDimmingDown
	
	if (aSceneSelectorKeyDimmingDown == AC_KEY_DOWN) // long press
	{
		addSceneSelector_clearKeyData();
		aSceneSelectorKeyDimmingDown = AC_KEY_DOWN;
	} else if (aSceneSelectorKeyDimmingDown == AC_KEY_UP_600MS_TO_1S)
	{
		addSceneSelector_clearKeyData();
		if (acDoubleClickCounter_whenKeyUp <= 1) // short press
		{
			addSceneSelector_sendDestinationAddressConfigure(aSceneSelectorCurrentEndpoint);
			zclGeneral_SendLevelControlStep(aSceneSelectorCurrentEndpoint, &zclSceneSelector_DstAddr, LEVEL_STEP_DOWN, 35, 1, FALSE, 0 );
		} else // double click
		{
			addSceneSelector_sendDestinationAddressConfigure(aSceneSelectorCurrentEndpoint);
			zclGeneral_SendOnOff_CmdOff(aSceneSelectorCurrentEndpoint, &zclSceneSelector_DstAddr, FALSE, 0 );
		}
	} else if ((aSceneSelectorKeyDimmingDown == AC_KEY_UP_1S_TO_5S) || (aSceneSelectorKeyDimmingDown == AC_KEY_UP_5S_TO_10S) || (aSceneSelectorKeyDimmingDown == AC_KEY_UP_ABOVE_10S))
	{
		addSceneSelector_clearKeyData();
		addSceneSelector_sendDestinationAddressConfigure(aSceneSelectorCurrentEndpoint);
		zclGeneral_SendLevelControlStop(aSceneSelectorCurrentEndpoint, &zclSceneSelector_DstAddr, FALSE, 0 );
	}
	
	else // others
		
	{
		addSceneSelector_clearKeyData();
	}
	
	acDoubleClickCounter_whenKeyUp = 0;
	acDoubleClickCounter_whenKeyDown = 0;
}

/*********************************************************************
 * @fn      addSceneSelector_isrPort0
 *
 * @brief   Port0 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addSceneSelector_isrPort0(void)
{
#if defined(SCENE_SELECTOR)	
	IEN1 &= ~0x20; // port0 interrupt disable

	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P0INT_VECTOR    
		//__interrupt void P0_ISR(void) 
		{
			// factory reset
			if (aSceneSelectorFactoryReset == ADD_SCENESELECTOR_FACTORYRESET_KEYDOWN_10S)
			{
				osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
			} else if (aSceneSelectorFactoryReset == ADD_SCENESELECTOR_FACTORYRESET_KEYDOWN_5S)
			{
				addSceneSelector_clearKeyData();
				acDoubleClickCounter_whenKeyUp = 0;
				aSceneSelectorFactoryReset = ADD_SCENESELECTOR_FACTORYRESET_KEYNORMAL;
				// no operation
			}

			else // P0_4(key scene1)
			
			if ((P0IFG & 0x10) == 0x10) 
			{
				if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_SCENESELECTOR_KEY_SCENE1 == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_SCENESELECTOR_KEY_SCENE1 == 0))
					{
						aSceneSelectorKeyScene1 = addCommon_handleKeyDown();
						P0IFG &= ~0x10; // clear interrupt flag
						P0IF = 0;
						EA = 0;
						PICTL &= ~0x01;
						acInterruptTriggerEdgePort0 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_SCENESELECTOR_KEY_SCENE1 == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_SCENESELECTOR_KEY_SCENE1 == 1))
					{	
						aSceneSelectorKeyScene1 = addCommon_handleKeyUp();
						if (aSceneSelectorKeyScene1 == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aSceneSelectorKeyScene1 = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aSceneSelectorKeyScene1 = AC_KEY_UP_600MS_TO_1S;
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

			else // P0_5(key scene2)
			
			if ((P0IFG & 0x20) == 0x20) 
			{
				if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_SCENESELECTOR_KEY_SCENE2 == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_SCENESELECTOR_KEY_SCENE2 == 0))
					{
						aSceneSelectorKeyScene2 = addCommon_handleKeyDown();
						P0IFG &= ~0x20; // clear interrupt flag
						P0IF = 0;
						EA = 0;
						PICTL &= ~0x01;
						acInterruptTriggerEdgePort0 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_SCENESELECTOR_KEY_SCENE2 == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_SCENESELECTOR_KEY_SCENE2 == 1))
					{	
						aSceneSelectorKeyScene2 = addCommon_handleKeyUp();
						if (aSceneSelectorKeyScene2 == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aSceneSelectorKeyScene2 = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aSceneSelectorKeyScene2 = AC_KEY_UP_600MS_TO_1S;
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

			else // P0_0(key scene3)
			
			if ((P0IFG & 0x01) == 0x01) 
			{
				if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_SCENESELECTOR_KEY_SCENE3 == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_SCENESELECTOR_KEY_SCENE3 == 0))
					{
						aSceneSelectorKeyScene3 = addCommon_handleKeyDown();
						P0IFG &= ~0x01; // clear interrupt flag
						P0IF = 0;
						EA = 0;
						PICTL &= ~0x01;
						acInterruptTriggerEdgePort0 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_SCENESELECTOR_KEY_SCENE3 == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort0 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_SCENESELECTOR_KEY_SCENE3 == 1))
					{	
						aSceneSelectorKeyScene3 = addCommon_handleKeyUp();
						if (aSceneSelectorKeyScene3 == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aSceneSelectorKeyScene3 = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aSceneSelectorKeyScene3 = AC_KEY_UP_600MS_TO_1S;
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
		
#elif defined(SCENE_SELECTOR_TOUCHKEY)
		
#else
#endif	
}

/*********************************************************************
 * @fn      addSceneSelector_isrPort1
 *
 * @brief   Port1 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addSceneSelector_isrPort1(void)
{
#if defined(SCENE_SELECTOR)	
	IEN2 &= ~0x10; // port1 interrupt disable	
	
	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P1INT_VECTOR    
		//__interrupt void P1_ISR(void) 
		{
			// wakeup
			if (aSceneSelectorWakeup == ADD_SCENESELECTOR_WAKEUP_KEYDOWN_10S)
			{
				osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
			} else if (aSceneSelectorWakeup == ADD_SCENESELECTOR_WAKEUP_KEYDOWN_5S)
			{
				addSceneSelector_clearKeyData();
				acDoubleClickCounter_whenKeyUp = 0;
				aSceneSelectorWakeup = ADD_SCENESELECTOR_WAKEUP_KEYNORMAL;
				// no operation
			}

			else // P1_6(key dimming up)
			
			if ((P1IFG & 0x40) == 0x40) 
			{
				if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_SCENESELECTOR_KEY_DIMMINGUP == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_SCENESELECTOR_KEY_DIMMINGUP == 0))
					{
						aSceneSelectorKeyDimmingUp = addCommon_handleKeyDown();
						P1IFG &= ~0x40; // clear interrupt flag
						P1IF = 0;
						EA = 0;
						PICTL &= ~0x06;
						acInterruptTriggerEdgePort1 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_SCENESELECTOR_KEY_DIMMINGUP == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_SCENESELECTOR_KEY_DIMMINGUP == 1))
					{
						aSceneSelectorKeyDimmingUp = addCommon_handleKeyUp();
						if (aSceneSelectorKeyDimmingUp == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aSceneSelectorKeyDimmingUp = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aSceneSelectorKeyDimmingUp = AC_KEY_UP_600MS_TO_1S;
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

			else // P1_7(key dimming down)
			
			if ((P1IFG & 0x80) == 0x80) 
			{
				if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_SCENESELECTOR_KEY_DIMMINGDOWN == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_SCENESELECTOR_KEY_DIMMINGDOWN == 0))
					{
						aSceneSelectorKeyDimmingDown = addCommon_handleKeyDown();
						P1IFG &= ~0x80; // clear interrupt flag
						P1IF = 0;
						EA = 0;
						PICTL &= ~0x06;
						acInterruptTriggerEdgePort1 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_SCENESELECTOR_KEY_DIMMINGDOWN == 1))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort1 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_SCENESELECTOR_KEY_DIMMINGDOWN == 1))
					{
						aSceneSelectorKeyDimmingDown = addCommon_handleKeyUp();
						if (aSceneSelectorKeyDimmingDown == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aSceneSelectorKeyDimmingDown = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aSceneSelectorKeyDimmingDown = AC_KEY_UP_600MS_TO_1S;
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
		
#elif defined(SCENE_SELECTOR_TOUCHKEY)
		
#else
#endif
}

/*********************************************************************
 * @fn      addSceneSelector_isrPort2
 *
 * @brief   Port2 interrupt service routine.
 *
 * @param   none
 *
 * @return  none
 */
void addSceneSelector_isrPort2(void)
{
#if defined(SCENE_SELECTOR)	
	IEN2 &= ~0x02; // port2 interrupt disable	
	
	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P2INT_VECTOR    
		//__interrupt void P2_ISR(void) 
		{
			// P2_0(key scene4)
			if ((P2IFG & 0x01) == 0x01)
			{	
				if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_SCENESELECTOR_KEY_SCENE4 == 0))
				{	
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_SCENESELECTOR_KEY_SCENE4 == 0))
					{
						aSceneSelectorKeyScene4 = addCommon_handleKeyDown();
						P2IFG &= ~0x01; // clear interrupt flag
						P2IF = 0;
						EA = 0;
						PICTL &= ~0x08;
						acInterruptTriggerEdgePort2 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_SCENESELECTOR_KEY_SCENE4 == 1))
				{	
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_SCENESELECTOR_KEY_SCENE4 == 1))
					{
						aSceneSelectorKeyScene4 = addCommon_handleKeyUp();
						if (aSceneSelectorKeyScene4 == AC_KEY_UP_BELOW_600MS)
						{
							if (acDoubleClickCheckFlag == TRUE)
							{
								aSceneSelectorKeyScene4 = AC_KEY_UP_600MS_TO_1S;
								acDoubleClickCounter_whenKeyUp++;
							} else 
							{
								aSceneSelectorKeyScene4 = AC_KEY_UP_600MS_TO_1S;
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
	
#elif defined(SCENE_SELECTOR_TOUCHKEY)
	IEN2 &= ~0x02; // port2 interrupt disable
	
	if (TRUE == addCommon_atNormalWorkState(FALSE))
	{
		//#pragma vector = P2INT_VECTOR    
		//__interrupt void P2_ISR(void) 
		{
			if ((P2IFG & 0x01) == 0x01) // P2_0 interrupt
			{
				if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
					(ADD_SCENESELECTOR_APT8L08_TP_INT == 0))
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_FALL) && 
						(ADD_SCENESELECTOR_APT8L08_TP_INT == 0))
					{
						aSceneSelectorApt8l08_16bitKeyMap_saved = 0;
						aSceneSelectorApt8l08_16bitKeyMap = 
							addSceneSelector_APT8L08_I2C_readByte(ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_KVR1) * 256 
							+ addSceneSelector_APT8L08_I2C_readByte(ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_KVR0);

						if (aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE1)
						{
							aSceneSelectorKeyScene1 = addCommon_handleKeyDown();
						} else if (aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE2)
						{
							aSceneSelectorKeyScene2 = addCommon_handleKeyDown();
						} else if (aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE3)
						{
							aSceneSelectorKeyScene3 = addCommon_handleKeyDown();
						} else if (aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE4)
						{
							aSceneSelectorKeyScene4 = addCommon_handleKeyDown();
						} else if (aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_DIMMINGUP)
						{
							aSceneSelectorKeyDimmingUp = addCommon_handleKeyDown();
						} else if (aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_DIMMINGDOWN)
						{
							aSceneSelectorKeyDimmingDown = addCommon_handleKeyDown();
						}						
						P2IFG &= ~0x01; // clear interrupt flag
						P2IF = 0;
						EA = 0;
						PICTL &= ~0x08;
						acInterruptTriggerEdgePort2 = AC_INTERRUPT_TRIGGER_EDGE_RISE;
						EA = 1;
					}
				} else if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
					(ADD_SCENESELECTOR_APT8L08_TP_INT == 1))//抬起后的中断处理，计算时间
				{
					MicroWait(10000);
					if ((acInterruptTriggerEdgePort2 == AC_INTERRUPT_TRIGGER_EDGE_RISE) && 
						(ADD_SCENESELECTOR_APT8L08_TP_INT == 1))
					{
						aSceneSelectorApt8l08_16bitKeyMap_saved = aSceneSelectorApt8l08_16bitKeyMap;
						aSceneSelectorApt8l08_16bitKeyMap = 
							addSceneSelector_APT8L08_I2C_readByte(ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_KVR1) * 256 
							+ addSceneSelector_APT8L08_I2C_readByte(ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_KVR0);
						
						osal_stop_timerEx(*acPointer_TaskID, AC_EVENT_DEVICE_KEY_TIMING);

						// factory reset
						if (aSceneSelectorFactoryReset == ADD_SCENESELECTOR_FACTORYRESET_KEYDOWN_10S)
						{
							osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
						} else if (aSceneSelectorFactoryReset == ADD_SCENESELECTOR_FACTORYRESET_KEYDOWN_5S)
						{
							addSceneSelector_clearKeyData();
							acDoubleClickCounter_whenKeyUp = 0;
							aSceneSelectorFactoryReset = ADD_SCENESELECTOR_FACTORYRESET_KEYNORMAL;
							// no operation
						}

						else // wakeup

						if (aSceneSelectorWakeup == ADD_SCENESELECTOR_WAKEUP_KEYDOWN_10S)
						{
							osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
						} else if (aSceneSelectorWakeup == ADD_SCENESELECTOR_WAKEUP_KEYDOWN_5S)
						{
							addSceneSelector_clearKeyData();
							acDoubleClickCounter_whenKeyUp = 0;
							aSceneSelectorWakeup = ADD_SCENESELECTOR_WAKEUP_KEYNORMAL;
							// no operation
						}

						else // key0

						if ((aSceneSelectorApt8l08_16bitKeyMap_saved & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE1)
							&& (!(aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE1)))
						{
							aSceneSelectorKeyScene1 = addCommon_handleKeyUp();
							if (aSceneSelectorKeyScene1 == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aSceneSelectorKeyScene1 = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aSceneSelectorKeyScene1 = AC_KEY_UP_600MS_TO_1S;
									osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
								}
							} else 
							{
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} 

						else // key1

						if ((aSceneSelectorApt8l08_16bitKeyMap_saved & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE2)
							&& (!(aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE2)))
						{
							aSceneSelectorKeyScene2 = addCommon_handleKeyUp();
							if (aSceneSelectorKeyScene2 == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aSceneSelectorKeyScene2 = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aSceneSelectorKeyScene2 = AC_KEY_UP_600MS_TO_1S;
									osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
								}
							} else 
							{
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} 

						else // key2

						if ((aSceneSelectorApt8l08_16bitKeyMap_saved & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE3)
							&& (!(aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE3)))
						{
							aSceneSelectorKeyScene3 = addCommon_handleKeyUp();
							if (aSceneSelectorKeyScene3 == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aSceneSelectorKeyScene3 = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aSceneSelectorKeyScene3 = AC_KEY_UP_600MS_TO_1S;
									osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
								}
							} else 
							{
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} 

						else // key3

						if ((aSceneSelectorApt8l08_16bitKeyMap_saved & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE4)
							&& (!(aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE4)))
						{
							aSceneSelectorKeyScene4 = addCommon_handleKeyUp();
							if (aSceneSelectorKeyScene4 == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aSceneSelectorKeyScene4 = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aSceneSelectorKeyScene4 = AC_KEY_UP_600MS_TO_1S;
									osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
								}
							} else 
							{
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} 

						else // keyDimmingUp

						if ((aSceneSelectorApt8l08_16bitKeyMap_saved & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_DIMMINGUP)
							&& (!(aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_DIMMINGUP)))
						{
							aSceneSelectorKeyDimmingUp = addCommon_handleKeyUp();
							if (aSceneSelectorKeyDimmingUp == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aSceneSelectorKeyDimmingUp = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aSceneSelectorKeyDimmingUp = AC_KEY_UP_600MS_TO_1S;
									osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
								}
							} else 
							{
								osal_set_event(*acPointer_TaskID, AC_EVENT_HANDLE_ONE_TIME);
							}
						} 

						else // keyDimmingDown

						if ((aSceneSelectorApt8l08_16bitKeyMap_saved & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_DIMMINGDOWN)
							&& (!(aSceneSelectorApt8l08_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_DIMMINGDOWN)))
						{
							aSceneSelectorKeyDimmingDown = addCommon_handleKeyUp();
							if (aSceneSelectorKeyDimmingDown == AC_KEY_UP_BELOW_600MS)
							{
								if (acDoubleClickCheckFlag == TRUE)
								{
									aSceneSelectorKeyDimmingDown = AC_KEY_UP_600MS_TO_1S;
									acDoubleClickCounter_whenKeyUp++;
								} else 
								{
									aSceneSelectorKeyDimmingDown = AC_KEY_UP_600MS_TO_1S;
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
 * @fn      addSceneSelector_sendDestinationAddressConfigure
 *
 * @brief   Destination address configure.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_sendDestinationAddressConfigure(uint8 endPoint)
{
	zclSceneSelector_DstAddr.addrMode = (afAddrMode_t)AddrGroup;
	zclSceneSelector_DstAddr.endPoint = endPoint;
	if (endPoint == SCENESELECTOR_ENDPOINT1)
	{
		zclSceneSelector_DstAddr.addr.shortAddr = aSceneSelectorEndpoint1_GroupID;
	} else if (endPoint == SCENESELECTOR_ENDPOINT2)
	{
		zclSceneSelector_DstAddr.addr.shortAddr = aSceneSelectorEndpoint2_GroupID;
	} else if (endPoint == SCENESELECTOR_ENDPOINT3)
	{
		zclSceneSelector_DstAddr.addr.shortAddr = aSceneSelectorEndpoint3_GroupID;
	} else if (endPoint == SCENESELECTOR_ENDPOINT4)
	{
		zclSceneSelector_DstAddr.addr.shortAddr = aSceneSelectorEndpoint4_GroupID;
	}

/*
uint16 groupList[APS_MAX_GROUPS];
uint8 numberOfGroups = 0;

uint16 *p = &(groupList[0]);
	for (int i = 0; i < APS_MAX_GROUPS; i++)
	{
		groupList[i] = 0x0000;
	}	
	
	numberOfGroups = aps_FindAllGroupsForEndpoint(endpoint, p);
	if (numberOfGroups == 0)
	{
		zclGeneral_SendSceneRecall(endpoint, &zclSceneSelector_DstAddr, 0x0000, sceneID, FALSE, 0);
	} else
	{
		zclGeneral_SendSceneRecall(endpoint, &zclSceneSelector_DstAddr, groupList[0], sceneID, FALSE, 0);
	}
*/	
}

/*********************************************************************
 * @fn      addSceneSelector_sendLevelControlMove
 *
 * @brief   Scene selector.
 *
 * @param   none
 *
 * @return  none
 */
void addSceneSelector_sendLevelControlMove(void)
{	
	if (aSceneSelectorKeyDimmingUp == AC_KEY_DOWN)
	{
		addSceneSelector_sendDestinationAddressConfigure(aSceneSelectorCurrentEndpoint);
		zclGeneral_SendLevelControlMove(aSceneSelectorCurrentEndpoint, &zclSceneSelector_DstAddr, LEVEL_MOVE_UP, 45, FALSE, 0 );
	} else if (aSceneSelectorKeyDimmingDown == AC_KEY_DOWN)
	{
		addSceneSelector_sendDestinationAddressConfigure(aSceneSelectorCurrentEndpoint);
		zclGeneral_SendLevelControlMove(aSceneSelectorCurrentEndpoint, &zclSceneSelector_DstAddr, LEVEL_MOVE_DOWN, 45, FALSE, 0 );
	}	
}

/*********************************************************************
 * @fn      addSceneSelector_clearKeyData
 *
 * @brief   Scene selector.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_clearKeyData(void)
{
	aSceneSelectorKeyScene1 = AC_KEY_NORMAL;
	aSceneSelectorKeyScene2 = AC_KEY_NORMAL;		
	aSceneSelectorKeyScene3 = AC_KEY_NORMAL;		
	aSceneSelectorKeyScene4 = AC_KEY_NORMAL;		
	aSceneSelectorKeyDimmingUp = AC_KEY_NORMAL;		
	aSceneSelectorKeyDimmingDown = AC_KEY_NORMAL;
}

/*********************************************************************
 * @fn      addSceneSelector_factoryResetCheck
 *
 * @brief   Scene selector.
 *
 * @param   none
 *
 * @return  none
 */
void addSceneSelector_factoryResetCheck(void)
{	
	uint16 temp_16bitKeyMap = 0;

	temp_16bitKeyMap = 
		addSceneSelector_APT8L08_I2C_readByte(ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_KVR1) * 256 
		+ addSceneSelector_APT8L08_I2C_readByte(ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_KVR0);
	
	if (acDeviceKeyTimingCounter == 50) // 5000ms
	{
		if ((temp_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE1)
			&& (temp_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE2))
		{
			aSceneSelectorFactoryReset = ADD_SCENESELECTOR_FACTORYRESET_KEYDOWN_5S;
		}
	} else if (acDeviceKeyTimingCounter == 100) // 10000ms
	{
		if ((aSceneSelectorFactoryReset == ADD_SCENESELECTOR_FACTORYRESET_KEYDOWN_5S)
			&& (temp_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE1)
			&& (temp_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE2))
		{
			aSceneSelectorFactoryReset = ADD_SCENESELECTOR_FACTORYRESET_KEYDOWN_10S;
		}
	}
}

/*********************************************************************
 * @fn      addSceneSelector_wakeupCheck
 *
 * @brief   Scene selector.
 *
 * @param   none
 *
 * @return  none
 */
void addSceneSelector_wakeupCheck(void)
{	
	uint16 temp_16bitKeyMap = 0;

	temp_16bitKeyMap = 
		addSceneSelector_APT8L08_I2C_readByte(ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_KVR1) * 256 
		+ addSceneSelector_APT8L08_I2C_readByte(ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_KVR0);
	
	if (acDeviceKeyTimingCounter == 50) // 5000ms
	{
		if ((temp_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE3)
			&& (temp_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE4))
		{
			aSceneSelectorWakeup = ADD_SCENESELECTOR_WAKEUP_KEYDOWN_5S;
		}
	} else if (acDeviceKeyTimingCounter == 100) // 10000ms
	{
		if ((aSceneSelectorWakeup == ADD_SCENESELECTOR_WAKEUP_KEYDOWN_5S)
			&& (temp_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE3)
			&& (temp_16bitKeyMap & ADD_SCENESELECTOR_APT8L08_TOUCHKEY_SCENE4))
		{
			aSceneSelectorWakeup = ADD_SCENESELECTOR_WAKEUP_KEYDOWN_10S;
		}
	}
}

/*********************************************************************
 * @fn      addSceneSelector_setGroupID
 *
 * @brief   Scene selector.
 *
 * @param   none
 *
 * @return  none
 */
void addSceneSelector_setGroupID(uint8 endPoint, aps_Group_t *group)
{
	if (endPoint == SCENESELECTOR_ENDPOINT1)
	{
		aSceneSelectorEndpoint1_GroupID = group->ID;
		osal_nv_write(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT1_GROUPID, 0, 2, &aSceneSelectorEndpoint1_GroupID);
	} else if (endPoint == SCENESELECTOR_ENDPOINT2)
	{
		aSceneSelectorEndpoint2_GroupID = group->ID;
		osal_nv_write(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT2_GROUPID, 0, 2, &aSceneSelectorEndpoint2_GroupID);
	} else if (endPoint == SCENESELECTOR_ENDPOINT3)
	{
		aSceneSelectorEndpoint3_GroupID = group->ID;
		osal_nv_write(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT3_GROUPID, 0, 2, &aSceneSelectorEndpoint3_GroupID);
	} else if (endPoint == SCENESELECTOR_ENDPOINT4)
	{
		aSceneSelectorEndpoint4_GroupID = group->ID;
		osal_nv_write(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT4_GROUPID, 0, 2, &aSceneSelectorEndpoint4_GroupID);
	}
}

/*********************************************************************
 * @fn      addSceneSelector_setSceneID
 *
 * @brief   Scene selector.
 *
 * @param   none
 *
 * @return  none
 */
void addSceneSelector_setSceneID(uint8 endPoint, zclGeneral_Scene_t *scene)
{
	if (endPoint == SCENESELECTOR_ENDPOINT1)
	{
		aSceneSelectorEndpoint1_SceneID = scene->ID;
		osal_nv_write(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT1_SCENEID, 0, 2, &aSceneSelectorEndpoint1_SceneID);
	} else if (endPoint == SCENESELECTOR_ENDPOINT2)
	{
		aSceneSelectorEndpoint2_SceneID = scene->ID;
		osal_nv_write(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT2_SCENEID, 0, 2, &aSceneSelectorEndpoint2_SceneID);
	} else if (endPoint == SCENESELECTOR_ENDPOINT3)
	{
		aSceneSelectorEndpoint3_SceneID = scene->ID;
		osal_nv_write(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT3_SCENEID, 0, 2, &aSceneSelectorEndpoint3_SceneID);
	} else if (endPoint == SCENESELECTOR_ENDPOINT4)
	{
		aSceneSelectorEndpoint4_SceneID = scene->ID;
		osal_nv_write(ADD_SCENESELECTOR_NV_ITEM_ID_ENDPOINT4_SCENEID, 0, 2, &aSceneSelectorEndpoint4_SceneID);
	}
}

#if defined(SCENE_SELECTOR)	
#elif defined(SCENE_SELECTOR_TOUCHKEY)

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_init
 *
 * @brief   APT8L08 initial.
 *
 * @param   none
 *
 * @return  none
 */
void addSceneSelector_APT8L08_init(void)
{	
	addSceneSelector_APT8L08_initHal();
	addCommon_delayMS(500);
	addSceneSelector_APT8L08_initAl();
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_initHal
 *
 * @brief   APT8L08 initial hardware abstract layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_APT8L08_initHal(void)
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

	ADD_SCENESELECTOR_APT8L08_SDA = 1;
	ADD_SCENESELECTOR_APT8L08_SCL = 1;
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_initAl
 *
 * @brief   APT8L08 initial application layer.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_APT8L08_initAl(void)
{
	addCommon_delayMS(ADD_SCENESELECTOR_APT8L08_POWERON_DELAY);

	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_SYSCON, ADD_SCENESELECTOR_APT8L08_MODE_CONFIGURE);
	addCommon_delayMS(10);
	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_SYSCON, ADD_SCENESELECTOR_APT8L08_MODE_NORMAL);
	addCommon_delayMS(10);
	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_SYSCON, ADD_SCENESELECTOR_APT8L08_MODE_CONFIGURE);
	addCommon_delayMS(10);
	
	for (uint8 i = 0; i < 16; i++)
	{
		addSceneSelector_APT8L08_I2C_writeRegister(
			aSceneSelectorApt8l08_registerAddress[i], aSceneSelectorApt8l08_registerData[i]);
	}
	
	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_GSR, ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_GSR); //gsr
	addCommon_delayMS(10);
	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_MCON, ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_MCON); //mcon 当使用AT8L08时,高4位一定要配置为“5";当使用AT8L16时,高4位配置为“0”
	addCommon_delayMS(10);
	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_K0_ENB, ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_K0_ENB); //K0_CON
	addCommon_delayMS(10);
	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_K1_ENB, ADD_SCENESELECTOR_APT8L08_REGISTER_DATA_K1_ENB); //K1_CON
	addCommon_delayMS(10);

	//addSceneSelector_APT8L08_I2C_writeRegister(
	//		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_KOR00, aSceneSelectorApt8l08_tkFin8bit[0]);
	//	addCommon_delayMS(10);
	for (uint8 i = 0; i < 16; i++)
	{
		addSceneSelector_APT8L08_I2C_writeRegister(i, aSceneSelectorApt8l08_tkFin8bit[i]);
		addCommon_delayMS(10);
	}

	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_FILTER0, 0xac); //FILTER<1:0>, 分别对应原始值0 1 2 3 阶滤波；FILTER<3:2>，分别对应参考值寄存器更新时间2 4 8 16 次有效后更新； FILTER<5:4>, 分别对应参考值0 1 2 3 阶滤波ILTER<6>按键键值更新不设高上限
	addCommon_delayMS(10);
	//addSceneSelector_APT8L08_I2C_writeRegister(
	//	ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_SENSE_CON, 0x40);	//加快扫描速度
	//addCommon_delayMS(10);
	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_REF_UTH, 0x60); //参考寄存器更新阈值 
	addCommon_delayMS(10);
	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_KEY_ATH, 0xff); //开发模式寄存器 直接触摸PAD设置“0XFF",默认“0X10”
	addCommon_delayMS(10);
	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_MCONH, 0x42); //BIT 6 置“1” 去掉longpress 16s
	addCommon_delayMS(10);
	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_FILTER1, 0x00); //开发模式寄存器 直接触摸PAD设置“0X08",默认“0X00”
	addCommon_delayMS(10);
	addSceneSelector_APT8L08_I2C_writeRegister(
		ADD_SCENESELECTOR_APT8L08_REGISTER_ADDRESS_SYSCON, ADD_SCENESELECTOR_APT8L08_MODE_NORMAL);
	addCommon_delayMS(10);

	addCommon_delayMS(500);	
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_I2C_start
 *
 * @brief   APT8L08 i2c start.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_APT8L08_I2C_start(void)
{	
	ADD_SCENESELECTOR_APT8L08_SDA = 1;
	MicroWait(5);
	ADD_SCENESELECTOR_APT8L08_SCL = 1;
	MicroWait(5);
	ADD_SCENESELECTOR_APT8L08_SDA = 0;
	MicroWait(5);	
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_I2C_stop
 *
 * @brief   APT8L08 i2c stop.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_APT8L08_I2C_stop(void)
{	
	ADD_SCENESELECTOR_APT8L08_SDA = 0;
	MicroWait(5);
	ADD_SCENESELECTOR_APT8L08_SCL = 1;
	MicroWait(5);
	ADD_SCENESELECTOR_APT8L08_SDA = 1;
	MicroWait(5);	
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_I2C_waitACK
 *
 * @brief   APT8L08 i2c wait ACK.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_APT8L08_I2C_waitACK(void)
{
	uint8 bTemp;

	ADD_SCENESELECTOR_APT8L08_SCL = 1;
	ADD_SCENESELECTOR_APT8L08_SDA_DIR(ADD_SCENESELECTOR_INPUT);
	for (bTemp = 0; bTemp <= 250; bTemp++)
	{
		if (ADD_SCENESELECTOR_APT8L08_SDA)
		{
			// no operation
		} else 
		{
			break;
		}
	}

	if (bTemp >= 250)
	{
		aSceneSelectorApt8l08_i2cWaitStatus = 0x0a;
	}
	ADD_SCENESELECTOR_APT8L08_SCL = 0;
	ADD_SCENESELECTOR_APT8L08_SDA_DIR(ADD_SCENESELECTOR_OUTPUT);
	MicroWait(5);
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_I2C_noACK
 *
 * @brief   APT8L08 i2c no ACK.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_APT8L08_I2C_noACK(void)
{
	ADD_SCENESELECTOR_APT8L08_SDA = 1;
	MicroWait(5);
	ADD_SCENESELECTOR_APT8L08_SCL = 1;
	MicroWait(5);
	ADD_SCENESELECTOR_APT8L08_SCL = 0;
	MicroWait(5);
	ADD_SCENESELECTOR_APT8L08_SDA = 0;
	MicroWait(5);
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_I2C_ACK
 *
 * @brief   APT8L08 i2c ACK.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_APT8L08_I2C_ACK(void)
{
	ADD_SCENESELECTOR_APT8L08_SDA = 0;
	MicroWait(5);
	ADD_SCENESELECTOR_APT8L08_SCL = 1;
	MicroWait(5);
	ADD_SCENESELECTOR_APT8L08_SCL = 0;
	MicroWait(5);
	ADD_SCENESELECTOR_APT8L08_SDA = 1;
	MicroWait(5);
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_I2C_write8bit
 *
 * @brief   APT8L08 i2c write 8 bit.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_APT8L08_I2C_write8bit(uint8 dataWrite)
{
	ADD_SCENESELECTOR_APT8L08_SCL = 0;
	MicroWait(5);
	for (uint8 i = 0; i < 8; i++)
	{
		if ((dataWrite << i) & 0x80)
			ADD_SCENESELECTOR_APT8L08_SDA = 1;
		else			
			ADD_SCENESELECTOR_APT8L08_SDA = 0;
		MicroWait(5);
		ADD_SCENESELECTOR_APT8L08_SCL = 1;
		MicroWait(5);
		ADD_SCENESELECTOR_APT8L08_SCL = 0;
		MicroWait(5);		
	}
	ADD_SCENESELECTOR_APT8L08_SCL = 0;
	MicroWait(5);
	ADD_SCENESELECTOR_APT8L08_SDA = 1;
	MicroWait(5);	
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_I2C_read8bit
 *
 * @brief   APT8L08 i2c read 8 bit.
 *
 * @param   none
 *
 * @return  none
 */
static uint8 addSceneSelector_APT8L08_I2C_read8bit(void)
{	
	uint8 dataReceive;

	ADD_SCENESELECTOR_APT8L08_SDA_DIR(ADD_SCENESELECTOR_INPUT);
	for (uint8 i = 0; i < 8; i++)
	{
		dataReceive = dataReceive << 1;
		ADD_SCENESELECTOR_APT8L08_SCL = 1;
		MicroWait(5);
		if (ADD_SCENESELECTOR_APT8L08_SDA)
			dataReceive = dataReceive | 0x01;
		ADD_SCENESELECTOR_APT8L08_SCL = 0;
		MicroWait(5);		
	}
	ADD_SCENESELECTOR_APT8L08_SDA_DIR(ADD_SCENESELECTOR_OUTPUT);
	ADD_SCENESELECTOR_APT8L08_SCL = 0;
	ADD_SCENESELECTOR_APT8L08_SDA = 1;
	MicroWait(5);

	return dataReceive;
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_I2C_writeByte
 *
 * @brief   APT8L08 i2c write byte.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_APT8L08_I2C_writeByte(uint8 address, uint8 dataWrite)
{
	addSceneSelector_APT8L08_I2C_start();

	addSceneSelector_APT8L08_I2C_write8bit(0xac);
	addSceneSelector_APT8L08_I2C_waitACK();
	addSceneSelector_APT8L08_I2C_write8bit(address);
	addSceneSelector_APT8L08_I2C_waitACK();

	addSceneSelector_APT8L08_I2C_write8bit(dataWrite);
	addSceneSelector_APT8L08_I2C_waitACK();

	addSceneSelector_APT8L08_I2C_stop();
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_I2C_readByte
 *
 * @brief   APT8L08 i2c read byte.
 *
 * @param   none
 *
 * @return  none
 */
static uint8 addSceneSelector_APT8L08_I2C_readByte(uint8 address)
{
	uint8 dataReceive;
	
	addSceneSelector_APT8L08_I2C_start();

	addSceneSelector_APT8L08_I2C_write8bit(0xac);
	addSceneSelector_APT8L08_I2C_waitACK();
	addSceneSelector_APT8L08_I2C_write8bit(address);
	addSceneSelector_APT8L08_I2C_waitACK();

	addSceneSelector_APT8L08_I2C_start();

	addSceneSelector_APT8L08_I2C_write8bit(0xad);
	addSceneSelector_APT8L08_I2C_waitACK();

	dataReceive = addSceneSelector_APT8L08_I2C_read8bit();
	addSceneSelector_APT8L08_I2C_noACK();
	
	addSceneSelector_APT8L08_I2C_stop();

	return dataReceive;
}

/*********************************************************************
 * @fn      addSceneSelector_APT8L08_I2C_writeRegister
 *
 * @brief   APT8L08 i2c write register.
 *
 * @param   none
 *
 * @return  none
 */
static void addSceneSelector_APT8L08_I2C_writeRegister(uint8 address, uint8 dataWrite)
{
	do
	{
		addSceneSelector_APT8L08_I2C_writeByte(address, dataWrite);
	} while (dataWrite != addSceneSelector_APT8L08_I2C_readByte(address));
}
	
#else
#endif

