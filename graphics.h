#pragma once
#include <stdint.h>

/*======================================================================================*/
/* FONT                                                                                 */
/*======================================================================================*/

#define VIS_FONT_W 16
#define VIS_FONT_H 24
#define VIS_FONT_PX (VIS_FONT_W*VIS_FONT_H)

#define VIS_FONT_INDEX_0 0
#define VIS_FONT_INDEX_1 1
#define VIS_FONT_INDEX_2 2
#define VIS_FONT_INDEX_3 3
#define VIS_FONT_INDEX_4 4
#define VIS_FONT_INDEX_5 5
#define VIS_FONT_INDEX_6 6
#define VIS_FONT_INDEX_7 7
#define VIS_FONT_INDEX_8 8
#define VIS_FONT_INDEX_9 9
#define VIS_FONT_INDEX_UP_A 10
#define VIS_FONT_INDEX_UP_B 11
#define VIS_FONT_INDEX_UP_C 12
#define VIS_FONT_INDEX_UP_D 13
#define VIS_FONT_INDEX_UP_E 14
#define VIS_FONT_INDEX_UP_F 15
#define VIS_FONT_INDEX_UP_G 16
#define VIS_FONT_INDEX_UP_H 17
#define VIS_FONT_INDEX_UP_I 18
#define VIS_FONT_INDEX_UP_J 19
#define VIS_FONT_INDEX_UP_K 20
#define VIS_FONT_INDEX_UP_L 21
#define VIS_FONT_INDEX_UP_M 22
#define VIS_FONT_INDEX_UP_N 23
#define VIS_FONT_INDEX_UP_O 24
#define VIS_FONT_INDEX_UP_P 25
#define VIS_FONT_INDEX_UP_Q 26
#define VIS_FONT_INDEX_UP_R 27
#define VIS_FONT_INDEX_UP_S 28
#define VIS_FONT_INDEX_UP_T 29
#define VIS_FONT_INDEX_UP_U 30
#define VIS_FONT_INDEX_UP_V 31
#define VIS_FONT_INDEX_UP_W 32
#define VIS_FONT_INDEX_UP_X 33
#define VIS_FONT_INDEX_UP_Y 34
#define VIS_FONT_INDEX_UP_Z 35
#define VIS_FONT_INDEX_PERCENT 36
#define VIS_FONT_INDEX_DOT 37
#define VIS_FONT_INDEX_DEGC 38
#define VIS_FONT_INDEX_ARROW_R 39
#define VIS_FONT_INDEX_ARROW_L 40
#define VIS_FONT_INDEX_SELECT 41
#define VIS_FONT_INDEX_SPACE 42
#define VIS_FONT_INDEX_COLON 43
#define VIS_FONT_INDEX_SLASH 44
#define VIS_FONT_INDEX_BACKSLASH 45
#define VIS_FONT_INDEX_UNDERSCORE 46
#define VIS_FONT_INDEX_PLUS 47
#define VIS_FONT_INDEX_MINUS 48
#define VIS_FONT_INDEX_MULTIPLY 49
#define VIS_FONT_INDEX_EQUALS 50

#define VIS_FONT_INDEX_UNKNOWN 51

uint8_t char_to_font_index(char c);
extern uint16_t font[][VIS_FONT_PX];

#define VIS_SELECTBAR_W VIS_FONT_W
#define VIS_SELECTBAR_H 4
#define VIS_SELECTBAR_PX (VIS_SELECTBAR_H*VIS_SELECTBAR_W)
#define VIS_SELECTBAR_VOFFSET 4
#define VIS_SELECTBAR_NORM 0
#define VIS_SELECTBAR_NEG 1
#define VIS_SELECTBAR_NSEL 2

extern uint16_t selectbar[3][VIS_SELECTBAR_PX];