/**
 * \file
 * \brief This is a configuration file for the RNet stack
 * \author (c) 2013 Erich Styger, http://mcuoneclipse.com/
 * \note MIT License (http://opensource.org/licenses/mit-license.html)
 *
 * Here the stack can be configured using macros.
 */

#ifndef __RNET_APP_CONFIG__
#define __RNET_APP_CONFIG__

#include "Platform.h"
#if PL_CONFIG_HAS_RADIO
/*! type ID's for application messages */
typedef enum {
  RAPP_MSG_TYPE_STDIN = 0x00,
  RAPP_MSG_TYPE_STDOUT = 0x01,
  RAPP_MSG_TYPE_STDERR = 0x02,
  RAPP_MSG_TYPE_ACCEL = 0x03,
  RAPP_MSG_TYPE_DATA = 0x04,
  RAPP_MSG_TYPE_JOYSTICK_XY = 0x05,
  RAPP_MSG_TYPE_JOYSTICK_BTN = 0x54, /* Joystick button message (data is one byte: 'A', 'B', ... 'F' and 'K') */
  RAPP_MSG_TYPE_STOPP_ALL = 0x10,
  RAPP_MSG_TYPE_SPEED_INCREASE = 0x11,
  RAPP_MSG_TYPE_SPEED_DECREASE = 0x12,
  RAPP_MSG_TYPE_TURN_LEFTER = 0x13,
  RAPP_MSG_TYPE_TURN_RIGHTER = 0x14,
  RAPP_MSG_TYPE_REMOTE_ENABLE = 0x15,
  RAPP_MSG_TYPE_REMOTE_DISABLE = 0x16
  /* \todo extend with your own messages */
} RAPP_MSG_Type;

#define DEF_RADIO_ADDRESS_ROBOT 35
#define DEF_RADIO_ADDRESS_REMOTE 34

#endif /* PL_CONFIG_HAS_RADIO */

#endif /* __RNET_APP_CONFIG__ */
