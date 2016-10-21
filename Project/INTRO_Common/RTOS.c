/**
 * \file
 * \brief Real Time Operating System (RTOS) main program.
 * \author Erich Styger, erich.styger@hslu.ch
 */

#include "Platform.h"
#if PL_CONFIG_HAS_RTOS
#include "RTOS.h"
#include "FRTOS1.h"
#include "LED.h"
#include "Event.h"
#include "Keys.h"
#include "Application.h"

static void AppTask(void* param) {
  const int *whichLED = (int*)param;

  (void)param; /* avoid compiler warning */
  for(;;) {
//    if (*whichLED==1) {
//      LED1_Neg();
//    } else if (*whichLED==2) {
//      LED2_Neg();
//    }

#if PL_CONFIG_HAS_KEYS
    KEY_Scan();
#endif
#if PL_CONFIG_HAS_EVENTS
    EVNT_HandleEvent(APP_EventHandler, TRUE);
#endif

    /* \todo handle your application code here */
    FRTOS1_vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void RTOS_Init(void) {
  EVNT_SetEvent(EVNT_STARTUP); /* set startup event */

  RTOS_Run();			/*! \ Create tasks here */

}

void RTOS_Deinit(void) {
  /* nothing needed for now */
}

void RTOS_Run(void) {
	static const int led1 = 1;
	static const int led2 = 2;

	if (FRTOS1_xTaskCreate(AppTask, (signed portCHAR *)"App1", configMINIMAL_STACK_SIZE, (void*)&led1, tskIDLE_PRIORITY, NULL) != pdPASS) {
		for(;;){} /* error case only, stay here! */
	}
}


#endif /* PL_CONFIG_HAS_RTOS */
