#pragma once
#include <stdint.h>

/*======================================================================================*/
/* FONT                                                                                 */
/*======================================================================================*/

#define GUI_FONT_W 16
#define GUI_FONT_H 24
#define GUI_FONT_PX (GUI_FONT_W*GUI_FONT_H)

#define GUI_FONT_INDEX_0 0
#define GUI_FONT_INDEX_1 1
#define GUI_FONT_INDEX_2 2
#define GUI_FONT_INDEX_3 3
#define GUI_FONT_INDEX_4 4
#define GUI_FONT_INDEX_5 5
#define GUI_FONT_INDEX_6 6
#define GUI_FONT_INDEX_7 7
#define GUI_FONT_INDEX_8 8
#define GUI_FONT_INDEX_9 9
#define GUI_FONT_INDEX_UP_A 10
#define GUI_FONT_INDEX_UP_B 11
#define GUI_FONT_INDEX_UP_C 12
#define GUI_FONT_INDEX_UP_D 13
#define GUI_FONT_INDEX_UP_E 14
#define GUI_FONT_INDEX_UP_F 15
#define GUI_FONT_INDEX_UP_G 16
#define GUI_FONT_INDEX_UP_H 17
#define GUI_FONT_INDEX_UP_I 18
#define GUI_FONT_INDEX_UP_J 19
#define GUI_FONT_INDEX_UP_K 20
#define GUI_FONT_INDEX_UP_L 21
#define GUI_FONT_INDEX_UP_M 22
#define GUI_FONT_INDEX_UP_N 23
#define GUI_FONT_INDEX_UP_O 24
#define GUI_FONT_INDEX_UP_P 25
#define GUI_FONT_INDEX_UP_Q 26
#define GUI_FONT_INDEX_UP_R 27
#define GUI_FONT_INDEX_UP_S 28
#define GUI_FONT_INDEX_UP_T 29
#define GUI_FONT_INDEX_UP_U 30
#define GUI_FONT_INDEX_UP_V 31
#define GUI_FONT_INDEX_UP_W 32
#define GUI_FONT_INDEX_UP_X 33
#define GUI_FONT_INDEX_UP_Y 34
#define GUI_FONT_INDEX_UP_Z 35
#define GUI_FONT_INDEX_PERCENT 36
#define GUI_FONT_INDEX_DOT 37
#define GUI_FONT_INDEX_DEGC 38
#define GUI_FONT_INDEX_ARROW_R 39
#define GUI_FONT_INDEX_ARROW_L 40
#define GUI_FONT_INDEX_SELECT 41
#define GUI_FONT_INDEX_SPACE 42
#define GUI_FONT_INDEX_COLON 43
#define GUI_FONT_INDEX_SLASH 44
#define GUI_FONT_INDEX_BACKSLASH 45
#define GUI_FONT_INDEX_UNDERSCORE 46
#define GUI_FONT_INDEX_PLUS 47
#define GUI_FONT_INDEX_MINUS 48
#define GUI_FONT_INDEX_MULTIPLY 49
#define GUI_FONT_INDEX_EQUALS 50

#define GUI_FONT_INDEX_UNKNOWN 51

uint8_t char_to_font_index(char c);
extern uint16_t font[][GUI_FONT_PX];

#define GUI_SELECTBAR_W GUI_FONT_W
#define GUI_SELECTBAR_H 2
#define GUI_SELECTBAR_PX (GUI_SELECTBAR_H*GUI_SELECTBAR_W)
#define GUI_SELECTBAR_VOFFSET 2
#define GUI_SELECTBAR_NORM 0
#define GUI_SELECTBAR_NEG 1

extern uint16_t selectbar[2][GUI_SELECTBAR_PX];