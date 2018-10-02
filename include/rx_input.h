#ifndef RX_INPUT_H
#define RX_INPUT_H

/*
 * made by ekknod
 * github.com/ekknod
 *
 */

#include "rx_types.h"
#include "rx_handle.h"

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

typedef enum _RX_KEYCODE {
    RX_KEY_RESERVED         = 0,
    RX_KEY_ESC              = 1,
    RX_KEY_1                = 2,
    RX_KEY_2                = 3,
    RX_KEY_3                = 4,
    RX_KEY_4                = 5,
    RX_KEY_5                = 6,
    RX_KEY_6                = 7,
    RX_KEY_7                = 8,
    RX_KEY_8                = 9,
    RX_KEY_9                = 10,
    RX_KEY_0                = 11,
    RX_KEY_MINUS            = 12,
    RX_KEY_EQUAL            = 13,
    RX_KEY_BACKSPACE        = 14,
    RX_KEY_TAB              = 15,
    RX_KEY_Q                = 16,
    RX_KEY_W                = 17,
    RX_KEY_E                = 18,
    RX_KEY_R                = 19,
    RX_KEY_T                = 20,
    RX_KEY_Y                = 21,
    RX_KEY_U                = 22,
    RX_KEY_I                = 23,
    RX_KEY_O                = 24,
    RX_KEY_P                = 25,
    RX_KEY_LEFTBRACE        = 26,
    RX_KEY_RIGHTBRACE       = 27,
    RX_KEY_ENTER            = 28,
    RX_KEY_LEFTCTRL         = 29,
    RX_KEY_A                = 30,
    RX_KEY_S                = 31,
    RX_KEY_D                = 32,
    RX_KEY_F                = 33,
    RX_KEY_G                = 34,
    RX_KEY_H                = 35,
    RX_KEY_J                = 36,
    RX_KEY_K                = 37,
    RX_KEY_L                = 38,
    RX_KEY_SEMICOLON        = 39,
    RX_KEY_APOSTROPHE       = 40,
    RX_KEY_GRAVE            = 41,
    RX_KEY_LEFTSHIFT        = 42,
    RX_KEY_BACKSLASH        = 43,
    RX_KEY_Z                = 44,
    RX_KEY_X                = 45,
    RX_KEY_C                = 46,
    RX_KEY_V                = 47,
    RX_KEY_B                = 48,
    RX_KEY_N                = 49,
    RX_KEY_M                = 50,
    RX_KEY_COMMA            = 51,
    RX_KEY_DOT              = 52,
    RX_KEY_SLASH            = 53,
    RX_KEY_RIGHTSHIFT       = 54,
    RX_KEY_KPASTERISK       = 55,
    RX_KEY_LEFTALT          = 56,
    RX_KEY_SPACE            = 57,
    RX_KEY_CAPSLOCK         = 58,
    RX_KEY_F1               = 59,
    RX_KEY_F2               = 60,
    RX_KEY_F3               = 61,
    RX_KEY_F4               = 62,
    RX_KEY_F5               = 63,
    RX_KEY_F6               = 64,
    RX_KEY_F7               = 65,
    RX_KEY_F8               = 66,
    RX_KEY_F9               = 67,
    RX_KEY_F10              = 68,
    RX_KEY_NUMLOCK          = 69,
    RX_KEY_SCROLLLOCK       = 70,
    RX_KEY_KP7              = 71,
    RX_KEY_KP8              = 72,
    RX_KEY_KP9              = 73,
    RX_KEY_KPMINUS          = 74,
    RX_KEY_KP4              = 75,
    RX_KEY_KP5              = 76,
    RX_KEY_KP6              = 77,
    RX_KEY_KPPLUS           = 78,
    RX_KEY_KP1              = 79,
    RX_KEY_KP2              = 80,
    RX_KEY_KP3              = 81,
    RX_KEY_KP0              = 82,
    RX_KEY_KPDOT            = 83,
    RX_KEY_ZENKAKUHANKAKU   = 85,
    RX_KEY_102ND            = 86,
    RX_KEY_F11              = 87,
    RX_KEY_F12              = 88,
    RX_KEY_RO               = 89,
    RX_KEY_KATAKANA         = 90,
    RX_KEY_HIRAGANA         = 91,
    RX_KEY_HENKAN           = 92,
    RX_KEY_KATAKANAHIRAGANA = 93,
    RX_KEY_MUHENKAN         = 94,
    RX_KEY_KPJPCOMMA        = 95,
    RX_KEY_KPENTER          = 96,
    RX_KEY_RIGHTCTRL        = 97,
    RX_KEY_KPSLASH          = 98,
    RX_KEY_SYSRQ            = 99,
    RX_KEY_RIGHTALT         = 100,
    RX_KEY_LINEFEED         = 101,
    RX_KEY_HOME             = 102,
    RX_KEY_UP               = 103,
    RX_KEY_PAGEUP           = 104,
    RX_KEY_LEFT             = 105,
    RX_KEY_RIGHT            = 106,
    RX_KEY_END              = 107,
    RX_KEY_DOWN             = 108,
    RX_KEY_PAGEDOWN         = 109,
    RX_KEY_INSERT           = 110,
    RX_KEY_DELETE           = 111,
    RX_KEY_MACRO            = 112,
    RX_KEY_MUTE             = 113,
    RX_KEY_VOLUMEDOWN       = 114,
    RX_KEY_VOLUMEUP         = 115,
    RX_KEY_POWER            = 116,
    RX_KEY_KPEQUAL          = 117,
    RX_KEY_KPPLUSMINUS      = 118,
    RX_KEY_PAUSE            = 119,
    RX_KEY_SCALE            = 120,
    RX_KEY_KPCOMMA          = 121,
    RX_KEY_HANGEUL          = 122,
    RX_KEY_HANGUEL          = RX_KEY_HANGEUL,
    RX_KEY_HANJA            = 123,
    RX_KEY_YEN              = 124,
    RX_KEY_LEFTMETA         = 125,
    RX_KEY_RIGHTMETA        = 126,
    RX_KEY_COMPOSE          = 127,
    RX_KEY_STOP             = 128,
    RX_KEY_AGAIN            = 129,
    RX_KEY_PROPS            = 130,
    RX_KEY_UNDO             = 131,
    RX_KEY_FRONT            = 132,
    RX_KEY_COPY             = 133,
    RX_KEY_OPEN             = 134,
    RX_KEY_PASTE            = 135,
    RX_KEY_FIND             = 136,
    RX_KEY_CUT              = 137,
    RX_KEY_HELP             = 138,
    RX_KEY_MENU             = 139,
    RX_KEY_CALC             = 140,
    RX_KEY_SETUP            = 141,
    RX_KEY_SLEEP            = 142,
    RX_KEY_WAKEUP           = 143,
    RX_KEY_FILE             = 144,
    RX_KEY_SENDFILE         = 145,
    RX_KEY_DELETEFILE       = 146,
    RX_KEY_XFER             = 147,
    RX_KEY_PROG1            = 148,
    RX_KEY_PROG2            = 149,
    RX_KEY_WWW              = 150,
    RX_KEY_MSDOS            = 151,
    RX_KEY_COFFEE           = 152,
    RX_KEY_SCREENLOCK       = RX_KEY_COFFEE,
    RX_KEY_ROTATE_DISPLAY   = 153,
    RX_KEY_DIRECTION        = RX_KEY_ROTATE_DISPLAY,
    RX_KEY_CYCLEWINDOWS     = 154,
    RX_KEY_MAIL             = 155,
    RX_KEY_BOOKMARKS        = 156,
    RX_KEY_COMPUTER         = 157,
    RX_KEY_BACK             = 158,
    RX_KEY_FORWARD          = 159,
    RX_KEY_CLOSECD          = 160,
    RX_KEY_EJECTCD          = 161,
    RX_KEY_EJECTCLOSECD     = 162,
    RX_KEY_NEXTSONG         = 163,
    RX_KEY_PLAYPAUSE        = 164,
    RX_KEY_PREVIOUSSONG     = 165,
    RX_KEY_STOPCD           = 166,
    RX_KEY_RECORD           = 167,
    RX_KEY_REWIND           = 168,
    RX_KEY_PHONE            = 169,
    RX_KEY_ISO              = 170,
    RX_KEY_CONFIG           = 171,
    RX_KEY_HOMEPAGE         = 172,
    RX_KEY_REFRESH          = 173,
    RX_KEY_EXIT             = 174,
    RX_KEY_MOVE             = 175,
    RX_KEY_EDIT             = 176,
    RX_KEY_SCROLLUP         = 177,
    RX_KEY_SCROLLDOWN       = 178,
    RX_KEY_KPLEFTPAREN      = 179,
    RX_KEY_KPRIGHTPAREN     = 180,
    RX_KEY_NEW              = 181,
    RX_KEY_REDO             = 182,
    RX_KEY_F13              = 183,
    RX_KEY_F14              = 184,
    RX_KEY_F15              = 185,
    RX_KEY_F16              = 186,
    RX_KEY_F17              = 187,
    RX_KEY_F18              = 188,
    RX_KEY_F19              = 189,
    RX_KEY_F20              = 190,
    RX_KEY_F21              = 191,
    RX_KEY_F22              = 192,
    RX_KEY_F23              = 193,
    RX_KEY_F24              = 194,
    RX_KEY_PLAYCD           = 200,
    RX_KEY_PAUSECD          = 201,
    RX_KEY_PROG3            = 202,
    RX_KEY_PROG4            = 203,
    RX_KEY_DASHBOARD        = 204,
    RX_KEY_SUSPEND          = 205,
    RX_KEY_CLOSE            = 206,
    RX_KEY_PLAY             = 207,
    RX_KEY_FASTFORWARD      = 208,
    RX_KEY_BASSBOOST        = 209,
    RX_KEY_PRINT            = 210,
    RX_KEY_HP               = 211,
    RX_KEY_CAMERA           = 212,
    RX_KEY_SOUND            = 213,
    RX_KEY_QUESTION         = 214,
    RX_KEY_EMAIL            = 215,
    RX_KEY_CHAT             = 216,
    RX_KEY_SEARCH           = 217,
    RX_KEY_CONNECT          = 218,
    RX_KEY_FINANCE          = 219,
    RX_KEY_SPORT            = 220,
    RX_KEY_SHOP             = 221,
    RX_KEY_ALTERASE         = 222,
    RX_KEY_CANCEL           = 223,
    RX_KEY_BRIGHTNESSDOWN   = 224,
    RX_KEY_BRIGHTNESSUP     = 225,
    RX_KEY_MEDIA            = 226,
    RX_KEY_SWITCHVIDEOMODE  = 227,
    RX_KEY_KBDILLUMTOGGLE   = 228,
    RX_KEY_KBDILLUMDOWN     = 229,
    RX_KEY_KBDILLUMUP       = 230,
    RX_KEY_SEND             = 231,
    RX_KEY_REPLY            = 232,
    RX_KEY_FORWARDMAIL      = 233,
    RX_KEY_SAVE             = 234,
    RX_KEY_DOCUMENTS        = 235,
    RX_KEY_BATTERY          = 236,
    RX_KEY_BLUETOOTH        = 237,
    RX_KEY_WLAN             = 238,
    RX_KEY_UWB              = 239,
    RX_KEY_UNKNOWN          = 240,
    RX_KEY_VIDEO_NEXT       = 241,
    RX_KEY_VIDEO_PREV       = 242,
    RX_KEY_BRIGHTNESS_CYCLE = 243,
    RX_KEY_BRIGHTNESS_AUTO  = 244,
    RX_KEY_BRIGHTNESS_ZERO  = RX_KEY_BRIGHTNESS_AUTO,
    RX_KEY_DISPLAY_OFF      = 245,
    RX_KEY_WWAN             = 246,
    RX_KEY_WIMAX            = RX_KEY_WWAN,
    RX_KEY_RFKILL           = 247,
    RX_KEY_MICMUTE          = 248,
    RX_BTN_MISC             = 0x100,
    RX_BTN_0                = 0x100,
    RX_BTN_1                = 0x101,
    RX_BTN_2                = 0x102,
    RX_BTN_3                = 0x103,
    RX_BTN_4                = 0x104,
    RX_BTN_5                = 0x105,
    RX_BTN_6                = 0x106,
    RX_BTN_7                = 0x107,
    RX_BTN_8                = 0x108,
    RX_BTN_9                = 0x109,
    RX_BTN_MOUSE            = 0x110,
    RX_BTN_LEFT             = RX_BTN_MOUSE,
    RX_BTN_RIGHT            = 0x111,
    RX_BTN_MIDDLE           = 0x112,
    RX_BTN_SIDE             = 0x113,
    RX_BTN_EXTRA            = 0x114,
    RX_BTN_FORWARD          = 0x115,
    RX_BTN_BACK             = 0x116,
    RX_BTN_TASK             = 0x117,
    RX_KEYCODE_LAST         = RX_BTN_TASK
} RX_KEYCODE ;

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
rx_bool rx_key_down(
    _in_  rx_handle      input,
    _in_  RX_KEYCODE     key
    ) ;
#define rx_button_down rx_key_down
/*
 * RX_INPUT_MODE_RECEIVE / RX_INPUT_MODE_ALL needed
 */
vec2_i rx_input_axis(
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
void rx_send_input_key(
    _in_  rx_handle      input,
    _in_  RX_KEYCODE     key,
    _in_  rx_bool        down
    ) ;
#define rx_send_input_button rx_send_input_key

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RX_INPUT_H
