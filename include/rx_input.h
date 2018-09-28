#ifndef RX_INPUT_H
#define RX_INPUT_H

/*
 * made by ekknod
 * github.com/ekknod
 *
 */

#include "rx_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum _RX_INPUT_TYPE {
    RX_INPUT_TYPE_MOUSE    = 0,
    RX_INPUT_TYPE_KEYBOARD = 1
} RX_INPUT_TYPE ;

typedef enum _RX_INPUT_MODE {
    RX_INPUT_MODE_RECEIVE = 0,
    RX_INPUT_MODE_SEND    = 1,
    RX_INPUT_MODE_ALL     = 2
} RX_INPUT_MODE ;

typedef enum _RX_BUTTON_CODE {
    RX_BUTTON_MOUSE1 = 0,
    RX_BUTTON_MOUSE2 = 1,
    RX_BUTTON_MOUSE3 = 2,
    RX_BUTTON_MOUSE4 = 3,
    RX_BUTTON_MOUSE5 = 4
} RX_BUTTON_CODE ;

typedef enum _RX_MOUSE_AXIS {
    RX_MOUSE_X = 0,
    RX_MOUSE_Y = 1
} RX_MOUSE_AXIS ;

/*
 * RX_INPUT_TYPE_KEYBOARD is not implemented yet
 */
rx_handle rx_open_input(
    _in_  RX_INPUT_TYPE type,
    _in_  RX_INPUT_MODE mode
    ) ;

/*
 * RX_INPUT_MODE_RECEIVE / RX_INPUT_MODE_ALL needed
 */
rx_bool rx_button_down(
    _in_  rx_handle      mouse_input,
    _in_  RX_BUTTON_CODE button
    ) ;

/*
 * RX_INPUT_MODE_RECEIVE / RX_INPUT_MODE_ALL needed
 */
vec2_i rx_mouse_axis(
    _in_  rx_handle      mouse_input
    ) ;

/*
 * RX_INPUT_MODE_SEND / RX_INPUT_MODE_ALL needed
 */
void rx_send_input_axis(
    _in_  rx_handle      mouse_input,
    _in_  RX_MOUSE_AXIS  axis,
    _in_  int            px
    ) ;


/*
 * RX_INPUT_MODE_SEND / RX_INPUT_MODE_ALL needed
 */
void rx_send_input_button(
    _in_  rx_handle      mouse_input,
    _in_  RX_BUTTON_CODE button,
    _in_  rx_bool        down
    ) ;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RX_INPUT_H

