/**
 * \file
 * \brief Implements line following of the robot
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This is the implementation of the line following.
 */

#include "Platform.h"
#if PL_CONFIG_HAS_LINE_FOLLOW
#include "LineFollow.h"
#include "FRTOS1.h"
#include "CLS1.h"
#include "Shell.h"
#include "Motor.h"
#include "Reflectance.h"
#if PL_CONFIG_HAS_TURN
  #include "Turn.h"
#endif
#include "WAIT1.h"
#include "Pid.h"
#include "Drive.h"
#include "Shell.h"
#if PL_CONFIG_HAS_BUZZER
  #include "Buzzer.h"
#endif
#if PL_CONFIG_HAS_DRIVE
  #include "Drive.h"
#endif
#if PL_CONFIG_HAS_LINE_MAZE
  #include "Maze.h"
#endif

typedef enum {
  STATE_IDLE,              /* idle, not doing anything */
  STATE_FOLLOW_SEGMENT,    /* line following segment, going forward */
  STATE_TURN,              /* reached an intersection, turning around */
  STATE_FINISHED,          /* reached finish area */
  STATE_STOP,				/* stop the engines */
  STATE_STOP_CALM,
} StateType;

typedef enum{
	STOP,
	START,
	BREAK,
	FORWARD,
	TURN,
	BACK,
	END
}SwitchState;


/* task notification bits */
#define LF_START_FOLLOWING (1<<0)  /* start line following */
#define LF_STOP_FOLLOWING  (1<<1)  /* stop line following */

static void StateMachine2(void);
static bool FollowSegment(void);
static volatile StateType LF_currState = STATE_IDLE;
static volatile SwitchState FSM_state = STOP; //added by Kusi
static SwitchState FSM_state_save = START;  //store the current state
uint16_t lostcounter = 0;
bool writeflag = false;
static xTaskHandle LFTaskHandle;
#if PL_CONFIG_HAS_LINE_MAZE
static uint8_t LF_solvedIdx = 0; /*  index to iterate through the solution, zero is the solution start index */
#endif

void LF_StartFollowing(void) {
  (void)xTaskNotify(LFTaskHandle, LF_START_FOLLOWING, eSetBits);
}

void LF_StopFollowing(void) {
  (void)xTaskNotify(LFTaskHandle, LF_STOP_FOLLOWING, eSetBits);
}

void LF_StartStopFollowing(void){

	if((FSM_state==FORWARD)||(FSM_state==BACK)||(FSM_state==START)){
			(void)xTaskNotify(LFTaskHandle, LF_STOP_FOLLOWING, eSetBits);
	}

	if((FSM_state==STOP)||(FSM_state==END)||(FSM_state==BREAK)){
		 (void)xTaskNotify(LFTaskHandle, LF_START_FOLLOWING, eSetBits);
	}
}

//void LF_StartStopFollowing(void) {
// if((FSM_state==START)||(FSM_state==BREAK)){  //changed by Kusi
//	//if (LF_IsFollowing()) {
//    (void)xTaskNotify(LFTaskHandle, LF_STOP_FOLLOWING, eSetBits);
//  } else {
//    (void)xTaskNotify(LFTaskHandle, LF_START_FOLLOWING, eSetBits);
//  }
//}

void setBreak(void){
	FSM_state = BREAK;
}


static void StateMachine2(void){
	//case check stop

	switch(FSM_state){

	case STOP:
		LF_currState = STATE_IDLE;
		break;

	//case START:
		//FSM_state = FSM_state_save;  //go to saved state
	//	break;

	case BREAK:
		if(writeflag==FALSE){
			SHELL_SendString("BREAK!\r\n");
			writeflag=TRUE;
			LF_currState = STATE_STOP;
		}

		break;

	case START:
		//DRV_SetMode(DRV_MODE_SPEED);
		FSM_state = FORWARD;
		LF_currState = STATE_FOLLOW_SEGMENT;
		FSM_state_save = START;  //save current state
		break;

	case FORWARD:
		if(writeflag==FALSE){
			SHELL_SendString("FORWARD!\r\n");
			writeflag = TRUE;
			ContestSendSignal('B');
		}

		if(LF_currState == STATE_IDLE)
		{
			FSM_state = TURN;
			writeflag = FALSE;

		}


		break;

	case TURN:
		if(writeflag==FALSE){
			SHELL_SendString("TURN!\r\n");
			writeflag = TRUE;
			LF_currState = STATE_TURN;
		}
		FSM_state_save = FSM_state;  //save current state

		if(LF_currState == STATE_IDLE)
		{
			FSM_state = BACK;
			writeflag = FALSE;
		}
		break;

	case BACK:
		if(writeflag==FALSE){
			SHELL_SendString("BACK!\r\n");
			writeflag = TRUE;
			//DRV_SetMode(DRV_MODE_SPEED);
			LF_currState = STATE_FOLLOW_SEGMENT;
		}

		FSM_state_save = FSM_state;  //save current state

		if(LF_currState == STATE_IDLE)
		{
			FSM_state = END;
			writeflag = FALSE;
			DRV_Reset();
			DRV_SetMode(DRV_MODE_POS);
			DRV_SetPos(300, 300);
			ContestSendSignal('C');

		}
		break;

	case END:
		if(writeflag == FALSE){
		SHELL_SendString("END!\r\n");
		writeflag = TRUE;
		}
		if(DRV_IsStopped())
		{
			FSM_state = STOP;
			DRV_SetSpeed(0,0);			// changed by Kevin
			DRV_SetMode(DRV_MODE_STOP);
		}
		//(playtune!?)
		break;

	}/*end of switch*/
}/*end of function*/


//static void StateMachine(void);

/*!
 * \brief follows a line segment.
 * \return Returns TRUE if still on line segment
 */
static bool FollowSegment(void){
  uint16_t currLine;
  REF_LineKind currLineKind;

  currLine = REF_GetLineValue();
  currLineKind = REF_GetLineKind();
  if (currLineKind==REF_LINE_STRAIGHT) {
    PID_Line(currLine, REF_MIDDLE_LINE_VALUE); /* move along the line */
    return TRUE;
  } else {
    return FALSE; /* intersection/change of direction or not on line any more */
  }
}

static void StateMachine(void) {
  switch (LF_currState) {
    case STATE_IDLE:
      break;
    case STATE_FOLLOW_SEGMENT:
      if (!FollowSegment()) {
    #if PL_CONFIG_HAS_LINE_MAZE
        LF_currState = STATE_TURN; /* make turn */
        SHELL_SendString((unsigned char*)"no line, turn..\r\n");
    #else
        LF_currState = STATE_STOP; /* stop if we do not have a line any more */
        SHELL_SendString((unsigned char*)"No line, stopped!\r\n");
    #endif
      }
      break;

    case STATE_TURN:
      #if PL_CONFIG_HAS_LINE_MAZE
      /*! \todo Handle maze turning */
		#else
    	TURN_TurnAngle(180, NULL);    //changed by Kusi
    	TURN_Turn(TURN_STOP, NULL);	//changed by Kusi
    	//FSM_state = BACK;  //changed by Kusi
    	DRV_SetMode(DRV_MODE_NONE);			// changed by Kevin
    	LF_currState = STATE_IDLE;

		#endif /* PL_CONFIG_HAS_LINE_MAZE */
      break;

    case STATE_FINISHED:
      #if PL_CONFIG_HAS_LINE_MAZE
      /*! \todo Handle maze finished */
      #endif /* PL_CONFIG_HAS_LINE_MAZE */
      break;
    case STATE_STOP:
    	DRV_Stop(10);
    	DRV_SetMode(DRV_MODE_STOP);
      SHELL_SendString("Stopped!\r\n");

//#if PL_CONFIG_HAS_TURN
    //  TURN_Turn(TURN_STOP, NULL);  //commented by Kusi
//#endif
      LF_currState = STATE_IDLE;
      break;
  } /* switch */
}

bool LF_IsFollowing(void) {
  return LF_currState!=STATE_IDLE;
}

static void LineTask (void *pvParameters) {
  uint32_t notifcationValue;

  (void)pvParameters; /* not used */
  for(;;) {
    (void)xTaskNotifyWait(0UL, LF_START_FOLLOWING|LF_STOP_FOLLOWING, &notifcationValue, 0); /* check flags */
    if (notifcationValue&LF_START_FOLLOWING) {
      DRV_SetMode(DRV_MODE_NONE); /* disable any drive mode */
      PID_Start();
      if((FSM_state==BREAK)){
    	  FSM_state = FSM_state_save;
      }else{
    	  FSM_state = START;  //start
      }
      writeflag = FALSE;//continue, were you were stopped
     // LF_currState = STATE_FOLLOW_SEGMENT;  //changed by Kusi
    }
    if (notifcationValue&LF_STOP_FOLLOWING) {
    	FSM_state = BREAK;
     // LF_currState = STATE_STOP;  //changed by Kusi
    }
    StateMachine2();
    StateMachine();
    FRTOS1_vTaskDelay(5/portTICK_PERIOD_MS);
  }
}

static void LF_PrintHelp(const CLS1_StdIOType *io) {
  CLS1_SendHelpStr((unsigned char*)"line", (unsigned char*)"Group of line following commands\r\n", io->stdOut);
  CLS1_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows line help or status\r\n", io->stdOut);
  CLS1_SendHelpStr((unsigned char*)"  start|stop", (unsigned char*)"Starts or stops line following\r\n", io->stdOut);
}

static void LF_PrintStatus(const CLS1_StdIOType *io) {
  CLS1_SendStatusStr((unsigned char*)"line follow", (unsigned char*)"\r\n", io->stdOut);
  switch (LF_currState) {
    case STATE_IDLE: 
      CLS1_SendStatusStr((unsigned char*)"  state", (unsigned char*)"IDLE\r\n", io->stdOut);
      break;
    case STATE_FOLLOW_SEGMENT: 
      CLS1_SendStatusStr((unsigned char*)"  state", (unsigned char*)"FOLLOW_SEGMENT\r\n", io->stdOut);
      break;
    case STATE_STOP: 
      CLS1_SendStatusStr((unsigned char*)"  state", (unsigned char*)"STOP\r\n", io->stdOut);
      break;
    case STATE_TURN: 
      CLS1_SendStatusStr((unsigned char*)"  state", (unsigned char*)"TURN\r\n", io->stdOut);
      break;
    case STATE_FINISHED: 
      CLS1_SendStatusStr((unsigned char*)"  state", (unsigned char*)"FINISHED\r\n", io->stdOut);
      break;
    default: 
      CLS1_SendStatusStr((unsigned char*)"  state", (unsigned char*)"UNKNOWN\r\n", io->stdOut);
      break;
  } /* switch */
}

uint8_t LF_ParseCommand(const unsigned char *cmd, bool *handled, const CLS1_StdIOType *io) {
  uint8_t res = ERR_OK;

  if (UTIL1_strcmp((char*)cmd, (char*)CLS1_CMD_HELP)==0 || UTIL1_strcmp((char*)cmd, (char*)"line help")==0) {
    LF_PrintHelp(io);
    *handled = TRUE;
  } else if (UTIL1_strcmp((char*)cmd, (char*)CLS1_CMD_STATUS)==0 || UTIL1_strcmp((char*)cmd, (char*)"line status")==0) {
    LF_PrintStatus(io);
    *handled = TRUE;
  } else if (UTIL1_strcmp((char*)cmd, (char*)"line start")==0) {
    LF_StartFollowing();
    *handled = TRUE;
  } else if (UTIL1_strcmp((char*)cmd, (char*)"line stop")==0) {
    LF_StopFollowing();
    *handled = TRUE;
  }
  return res;
}

void LF_Deinit(void) {
  /* nothing needed */
}

void LF_Init(void) {
  FSM_state = STOP;
	//LF_currState = STATE_IDLE; changed by Kusi

  if (xTaskCreate(LineTask, "Line", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &LFTaskHandle) != pdPASS) {
    for(;;){} /* error */
  }
}
#endif /* PL_CONFIG_HAS_LINE_FOLLOW */
