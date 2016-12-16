/**
 * \file
 * \brief Main application file
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This provides the main application entry point.
 */

#include "Platform.h"
#include "Application.h"
#include "Event.h"
#include "LED.h"
#include "WAIT1.h"
#include "CS1.h"
#include "Keys.h"
#include "KIN1.h"
#include "Reflectance.h"  	//added by Kusi
#include "LCDMenu.h"		// added by Kevin
#include "RApp.h"			// added by Kevin
#include "RNet_AppConfig.h" // added by Kevin
#include "RNet_App.h"
#include "LineFollow.h"
#include "LCD.h"



#if PL_CONFIG_HAS_SHELL
  #include "CLS1.h"
#endif
#if PL_CONFIG_HAS_BUZZER
  #include "Buzzer.h"
#endif
#if PL_CONFIG_HAS_RTOS
  #include "FRTOS1.h"
  #include "RTOS.h"
#endif
#if PL_CONFIG_HAS_SHELL
  #include "Shell.h"
#endif
#if PL_CONFIG_HAS_QUADRATURE
  #include "Q4CLeft.h"
  #include "Q4CRight.h"
#endif
#if PL_CONFIG_HAS_MOTOR
  #include "Motor.h"
#endif
#if PL_CONFIG_BOARD_IS_ROBO_V2
  #include "PORT_PDD.h"
#endif


static bool remoteControlEnable = FALSE;
static bool startContestSignalA = FALSE;

#if PL_CONFIG_HAS_EVENTS
void APP_EventHandler(EVNT_Handle event) {

  uint8 dummyValue = 0;

  switch(event) {
  case EVNT_STARTUP:
    LED1_On(); /* just do something */
	#if PL_CONFIG_HAS_BUZZER
    BUZ_PlayTune(BUZ_TUNE_WELCOME);
	#endif
    break;

  case EVNT_LED_HEARTBEAT:
	  LED1_Neg();
	  break;

#if PL_CONFIG_HAS_KEYS

#if PL_CONFIG_NOF_KEYS>=1
  case EVNT_SW1_PRESSED:
    LED2_Neg();
#if PL_CONFIG_HAS_REFLECTANCE
    //REF_CalibrateStartStop(); //added by Kusi
#endif

#if PL_CONFIG_HAS_LINE_FOLLOW
    LF_StartStopFollowing();
#endif
    //CLS1_SendStr("SW1 pressed\r\n", CLS1_GetStdio()->stdOut);
    SHELL_SendString("SW1 pressed\r\n");  /*\todo disabled shell */
    #if PL_CONFIG_HAS_BUZZER
    BUZ_PlayTune(BUZ_TUNE_BUTTON);
    #endif
    if(!remoteControlEnable) {
		#if PL_CONFIG_HAS_LCD_MENU
    		LCDMenu_OnEvent(LCDMENU_EVENT_RIGHT, NULL);
		#endif
    }
    else{
		#if PL_CONFIG_BOARD_IS_REMOTE
    		(void)LCDMenu_IncDirection();
			(void)RAPP_SendPayloadDataBlock(&dummyValue, sizeof(dummyValue), RAPP_MSG_TYPE_TURN_RIGHTER, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
			LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL);
		#endif
    }
    break;
#endif

#if PL_CONFIG_NOF_KEYS>=2
  case EVNT_SW2_PRESSED:
     LED2_Neg();
     //CLS1_SendStr("SW2 pressed\r\n", CLS1_GetStdio()->stdOut);
     SHELL_SendString("SW2 pressed\r\n");  /*\todo disabled shell */
     #if PL_CONFIG_HAS_BUZZER
     BUZ_PlayTune(BUZ_TUNE_BUTTON);
     #endif
    if(!remoteControlEnable) {
		#if PL_CONFIG_HAS_LCD_MENU
    		LCDMenu_OnEvent(LCDMENU_EVENT_LEFT, NULL);
		#endif
    }
    else {
		#if PL_CONFIG_BOARD_IS_REMOTE
    		(void)LCDMenu_DecDirection();
    		(void)RAPP_SendPayloadDataBlock(&dummyValue, sizeof(dummyValue), RAPP_MSG_TYPE_TURN_LEFTER, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
    		LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL);
		#endif
    }
     break;
#endif

#if PL_CONFIG_NOF_KEYS>=3
  case EVNT_SW3_PRESSED:
    LED2_Neg();
    //CLS1_SendStr("SW3 pressed\r\n", CLS1_GetStdio()->stdOut);
    SHELL_SendString("SW3 pressed\r\n");  /*\todo disabled shell */
    #if PL_CONFIG_HAS_BUZZER
    BUZ_PlayTune(BUZ_TUNE_BUTTON);
    #endif
    if(!remoteControlEnable) {
		#if PL_CONFIG_HAS_LCD_MENU
		LCDMenu_OnEvent(LCDMENU_EVENT_DOWN, NULL);
		#endif
    }
    else {
		#if PL_CONFIG_BOARD_IS_REMOTE
    		(void)LCDMenu_DecSpeed();
			(void)RAPP_SendPayloadDataBlock(&dummyValue, sizeof(dummyValue), RAPP_MSG_TYPE_SPEED_DECREASE, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
			LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL);
		#endif
    }
    break;
#endif

#if PL_CONFIG_NOF_KEYS>=4
  case EVNT_SW4_PRESSED:
    LED2_Neg();
    //CLS1_SendStr("SW4 pressed\r\n", CLS1_GetStdio()->stdOut);
    SHELL_SendString("SW4 pressed\r\n");  /*\todo disabled shell */
    #if PL_CONFIG_HAS_BUZZER
    BUZ_PlayTune(BUZ_TUNE_BUTTON);
    #endif
    if(!remoteControlEnable) {
		#if PL_CONFIG_HAS_LCD_MENU
		LCDMenu_OnEvent(LCDMENU_EVENT_ENTER, NULL);
		#endif
    }
    else{
		#if PL_CONFIG_BOARD_IS_REMOTE
    		(void)LCDMenu_ResetDrive();
			(void)RAPP_SendPayloadDataBlock(&dummyValue, sizeof(dummyValue), RAPP_MSG_TYPE_STOPP_ALL, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
			LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL);
		#endif
    }
    break;
#endif


#if PL_CONFIG_NOF_KEYS>=5
  case EVNT_SW5_PRESSED:
    LED2_Neg();
    //CLS1_SendStr("SW5 pressed\r\n", CLS1_GetStdio()->stdOut);
    SHELL_SendString("SW5 pressed\r\n");  /*\todo disabled shell */
    #if PL_CONFIG_HAS_BUZZER
    BUZ_PlayTune(BUZ_TUNE_BUTTON);
    #endif
    if(!remoteControlEnable) {
		#if PL_CONFIG_HAS_LCD_MENU
		LCDMenu_OnEvent(LCDMENU_EVENT_UP, NULL);
		#endif
    }
    else {
		#if PL_CONFIG_BOARD_IS_REMOTE
    		LCDMenu_IncSpeed();
			(void)RAPP_SendPayloadDataBlock(&dummyValue, sizeof(dummyValue), RAPP_MSG_TYPE_SPEED_INCREASE, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
			LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL);
			if(!startContestSignalA) {
				ContestSendSignal('A');
			}
		#endif
    }
    break;
#endif

#if PL_CONFIG_NOF_KEYS>=6
  case EVNT_SW6_PRESSED:
    LED2_Neg();
    //CLS1_SendStr("SW6 pressed\r\n", CLS1_GetStdio()->stdOut);
    SHELL_SendString("SW6 pressed\r\n");  /*\todo disabled shell */
    #if PL_CONFIG_HAS_BUZZER
    BUZ_PlayTune(BUZ_TUNE_BUTTON);
    #endif
    if(!remoteControlEnable) {
		#if PL_CONFIG_HAS_LCD_MENU
		LCDMenu_OnEvent(LCDMENU_EVENT_DOWN, NULL);
		#endif
    }
#if PL_CONFIG_BOARD_IS_REMOTE
		(void)RAPP_SendPayloadDataBlock(&dummyValue, sizeof(dummyValue), RAPP_MSG_TYPE_REMOTE_DISABLE, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
		remoteControlEnable = FALSE;
	#endif
    break;
#endif

#if PL_CONFIG_NOF_KEYS>=7
  case EVNT_SW7_PRESSED:
    LED2_Neg();
    //CLS1_SendStr("SW7 pressed\r\n", CLS1_GetStdio()->stdOut);
    SHELL_SendString("SW7 pressed\r\n");  /*\todo disabled shell */
    #if PL_CONFIG_HAS_BUZZER
    BUZ_PlayTune(BUZ_TUNE_BUTTON);
    #endif
    if(!remoteControlEnable) {
		#if PL_CONFIG_HAS_LCD_MENU
		LCDMenu_OnEvent(LCDMENU_EVENT_UP, NULL);
		#endif
    }
	#if PL_CONFIG_BOARD_IS_REMOTE
		(void)RAPP_SendPayloadDataBlock(&dummyValue, sizeof(dummyValue), RAPP_MSG_TYPE_REMOTE_ENABLE, RNETA_GetDestAddr(), RPHY_PACKET_FLAGS_NONE);
		ContestSendSignal('T');
		remoteControlEnable = TRUE;
	#endif
    break;
#endif
#endif /* PL_CONFIG_HAS_KEYS */

    /* \todo extend handler as needed */
   } /* switch */
}
#endif /* PL_CONFIG_HAS_EVENTS */


void ContestSendSignal(char signal) {
	uint8_t message[2];
	message[0] = 8;

	switch(signal) {
		case 'A':
			message[1] = 'A';
			break;
		case 'B':
			message[1] = 'B';
			break;
		case 'C':
			message[1] = 'C';
			break;
		case 'T':
			message[1] = 'T';
			break;
		case 'X':
			message[1] = 'X';
			break;
		default:
			message[1] = 'C';
			break;
	}

	(void)RAPP_SendPayloadDataBlock(message, 2, 0xAC, 0x12, RPHY_PACKET_FLAGS_NONE);

}

static const KIN1_UID RoboIDs[] = {
  /* 0: L20, V2 */ {0x00,0x03,0x00,0x00,0x4E,0x45,0xB7,0x21,0x4E,0x45,0x32,0x15,0x30,0x02,0x00,0x13},
  /* 1: L21, V2 */ {0x00,0x05,0x00,0x00,0x4E,0x45,0xB7,0x21,0x4E,0x45,0x32,0x15,0x30,0x02,0x00,0x13},
  /* 2: L4, V1  */ {0x00,0x0B,0xFF,0xFF,0x4E,0x45,0xFF,0xFF,0x4E,0x45,0x27,0x99,0x10,0x02,0x00,0x24}, /* revert right motor */
};

static void APP_AdoptToHardware(void) {
  KIN1_UID id;
  uint8_t res;

  res = KIN1_UIDGet(&id);
  if (res!=ERR_OK) {
    for(;;); /* error */
  }
#if PL_CONFIG_HAS_MOTOR
  if (KIN1_UIDSame(&id, &RoboIDs[2])) { /* L4 */
    MOT_Invert(MOT_GetMotorHandle(MOT_MOTOR_LEFT), TRUE); /* revert left motor */

    #if PL_CONFIG_HAS_QUADRATURE
    (void)Q4CLeft_SwapPins(TRUE);
    (void)Q4CRight_SwapPins(TRUE);
	#endif
  }
#endif
#if PL_CONFIG_HAS_QUADRATURE && PL_CONFIG_BOARD_IS_ROBO_V2
  /* pull-ups for Quadrature Encoder Pins */
  PORT_PDD_SetPinPullSelect(PORTC_BASE_PTR, 10, PORT_PDD_PULL_UP);
  PORT_PDD_SetPinPullEnable(PORTC_BASE_PTR, 10, PORT_PDD_PULL_ENABLE);
  PORT_PDD_SetPinPullSelect(PORTC_BASE_PTR, 11, PORT_PDD_PULL_UP);
  PORT_PDD_SetPinPullEnable(PORTC_BASE_PTR, 11, PORT_PDD_PULL_ENABLE);
  PORT_PDD_SetPinPullSelect(PORTC_BASE_PTR, 16, PORT_PDD_PULL_UP);
  PORT_PDD_SetPinPullEnable(PORTC_BASE_PTR, 16, PORT_PDD_PULL_ENABLE);
  PORT_PDD_SetPinPullSelect(PORTC_BASE_PTR, 17, PORT_PDD_PULL_UP);
  PORT_PDD_SetPinPullEnable(PORTC_BASE_PTR, 17, PORT_PDD_PULL_ENABLE);
#endif
}


void APP_Start(void) {
#if PL_CONFIG_HAS_RTOS
#if configUSE_TRACE_HOOKS
  PTRC1_uiTraceStart();
#endif
#endif
  PL_Init();
#if PL_CONFIG_HAS_EVENTS
  EVNT_SetEvent(EVNT_STARTUP);
#endif
#if CLS1_DEFAULT_SERIAL
  CLS1_SendStr("Hello World!\r\n", CLS1_GetStdio()->stdOut);
#endif
  APP_AdoptToHardware();
#if PL_CONFIG_HAS_RTOS
  vTaskStartScheduler(); /* start the RTOS, create the IDLE task and run my tasks (if any) */
  /* does usually not return! */
#else
  for(;;) {
#if PL_CONFIG_HAS_KEYS
    KEY_Scan();
#endif
#if PL_CONFIG_HAS_EVENTS
    EVNT_HandleEvent(APP_EventHandler, TRUE);
#endif

    WAIT1_Waitms(100);/* just wait for some arbitrary time .... */
  }
#endif
}


