/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <pc.h>

#define MIN_EXT_SCAN_CODE 0x10

#define ARRAY_SIZEOF(x) (sizeof(x) / sizeof(x[0]))

/* A table containing ECMA-48 compatible encodings for extended keys.
   The encodings for unshifted arrow keys, F1-F12, Home, Insert, etc.
   match the encodings used by other x86 environments.  All others
   are invented for DJGPP.  */
static const unsigned char *
ecma48_encoding_table[] =
{
            /* 0x01: Alt-Escape */
            /* 0x0e: Alt-Backspace */
            /* 0x0f: Back Tab */
  "\e[71~", /* 0x10: Alt-Q */
  "\e[77~", /* 0x11: Alt-W */
  "\e[60~", /* 0x12: Alt-E */
  "\e[72~", /* 0x13: Alt-R */
  "\e[74~", /* 0x14: Alt-T */
  "\e[79~", /* 0x15: Alt-Y */
  "\e[75~", /* 0x16: Alt-U */
  "\e[64~", /* 0x17: Alt-I */
  "\e[69~", /* 0x18: Alt-O */
  "\e[70~", /* 0x19: Alt-P */
  0,        /* 0x1a: Alt-[ */
  0,        /* 0x1b: Alt-] */
  0,        /* 0x1c: Alt-Enter */
  0,        /* 0x1d */
  "\e[56~", /* 0x1e: Alt-A */
  "\e[73~", /* 0x1f: Alt-S */
  "\e[59~", /* 0x20: Alt-D */
  "\e[61~", /* 0x21: Alt-F */
  "\e[62~", /* 0x22: Alt-G */
  "\e[63~", /* 0x23: Alt-H */
  "\e[65~", /* 0x24: Alt-J */
  "\e[66~", /* 0x25: Alt-K */
  "\e[67~", /* 0x26: Alt-L */
  0,        /* 0x27: Alt-; */
  0,        /* 0x28: Alt-' */
  0,        /* 0x29: Alt-` */
  0,        /* 0x2a */
  0,        /* 0x2b: Alt-\ */
  "\e[80~", /* 0x2c: Alt-Z */
  "\e[78~", /* 0x2d: Alt-X */
  "\e[58~", /* 0x2e: Alt-C */
  "\e[76~", /* 0x2f: Alt-V */
  "\e[57~", /* 0x30: Alt-B */
  "\e[69~", /* 0x31: Alt-N */
  "\e[68~", /* 0x32: Alt-M */
  0,        /* 0x33: Alt-, */
  0,        /* 0x34: Alt-. */
  0,        /* 0x35: Alt-/ */
  0,        /* 0x36 */
  0,        /* 0x37: Alt-Keypad * */
  0,        /* 0x38 */
  0,        /* 0x39 */
  0,        /* 0x3a */
  "\e[[A",  /* 0x3b: F1 */
  "\e[[B",  /* 0x3c: F2 */
  "\e[[C",  /* 0x3d: F3 */
  "\e[[D",  /* 0x3e: F4 */
  "\e[[E",  /* 0x3f: F5 */
  "\e[17~", /* 0x40: F6 */
  "\e[18~", /* 0x41: F7 */
  "\e[19~", /* 0x42: F8 */
  "\e[20~", /* 0x43: F9 */
  "\e[21~", /* 0x44: F10 */
  0,        /* 0x45 */
  0,        /* 0x46 */
  "\e[1~",  /* 0x47: Home */
  "\e[A",   /* 0x48: Up Arrow */
  "\e[5~",  /* 0x49: Page Up */
  0,        /* 0x4a: Alt-Keypad - */
  "\e[D",   /* 0x4b: Left Arrow */
  0,        /* 0x4c */
  "\e[C",   /* 0x4d: Right Arrow */
  0,        /* 0x4e: Alt-Keypad + */
  "\e[4~",  /* 0x4f: End */
  "\e[B",   /* 0x50: Down Arrow */
  "\e[6~",  /* 0x51: Page Down */
  "\e[2~",  /* 0x52: Insert */
  "\e[3~",  /* 0x53: Delete */
  "\e[25~", /* 0x54: Shift-F1 */
  "\e[26~", /* 0x55: Shift-F2 */
  "\e[27~", /* 0x56: Shift-F3 */
  "\e[28~", /* 0x57: Shift-F4 */
  "\e[29~", /* 0x58: Shift-F5 */
  "\e[30~", /* 0x59: Shift-F6 */
  "\e[31~", /* 0x5a: Shift-F7 */
  "\e[32~", /* 0x5b: Shift-F8 */
  "\e[33~", /* 0x5c: Shift-F9 */
  "\e[34~", /* 0x5d: Shift-F10 */
  "\e[37~", /* 0x5e: Ctrl-F1 */
  "\e[38~", /* 0x5f: Ctrl-F2 */
  "\e[39~", /* 0x60: Ctrl-F3 */
  "\e[40~", /* 0x61: Ctrl-F4 */
  "\e[41~", /* 0x62: Ctrl-F5 */
  "\e[37~", /* 0x63: Ctrl-F6 */
  "\e[38~", /* 0x64: Ctrl-F7 */
  "\e[39~", /* 0x65: Ctrl-F8 */
  "\e[40~", /* 0x66: Ctrl-F9 */
  "\e[41~", /* 0x67: Ctrl-F10 */
  "\e[44~", /* 0x68: Alt-F1 */
  "\e[45~", /* 0x69: Alt-F2 */
  "\e[46~", /* 0x6a: Alt-F3 */
  "\e[47~", /* 0x6b: Alt-F4 */
  "\e[48~", /* 0x6c: Alt-F5 */
  "\e[49~", /* 0x6d: Alt-F6 */
  "\e[50~", /* 0x6e: Alt-F7 */
  "\e[51~", /* 0x6f: Alt-F8 */
  "\e[52~", /* 0x70: Alt-F9 */
  "\e[53~", /* 0x71: Alt-F10 */
  0,        /* 0x72: Ctrl-Print Screen */
  "\e[84~", /* 0x73: Ctrl-Left Arrow */
  "\e[83~", /* 0x74: Ctrl-Right Arrow */
  "\e[86~", /* 0x75: Ctrl-End */
  "\e[88~", /* 0x76: Ctrl-Page Down */
  "\e[85~", /* 0x77: Ctrl-Home */
  0,        /* 0x78: Alt-1 */
  0,        /* 0x79: Alt-2 */
  0,        /* 0x7a: Alt-3 */
  0,        /* 0x7b: Alt-4 */
  0,        /* 0x7c: Alt-5 */
  0,        /* 0x7d: Alt-6 */
  0,        /* 0x7e: Alt-7 */
  0,        /* 0x7f: Alt-8 */
  0,        /* 0x80: Alt-9 */
  0,        /* 0x81: Alt-0 */
  0,        /* 0x82: Alt-- */
  0,        /* 0x83: Alt-= */
  "\e[87~", /* 0x84: Ctrl-Page Up */
  "\e[23~", /* 0x85: F11 */
  "\e[24~", /* 0x86: F12 */
  "\e[35~", /* 0x87: Shift-F11 */
  "\e[36~", /* 0x88: Shift-F12 */
  "\e[42~", /* 0x89: Ctrl-F11 */
  "\e[43~", /* 0x8a: Ctrl-F12 */
  "\e[54~", /* 0x8b: Alt-F11 */
  "\e[55~", /* 0x8c: Alt-F12 */
  "\e[81~", /* 0x8d: Ctrl-Up Arrow */
  0,        /* 0x8e: Ctrl-Keypad - */
  0,        /* 0x8f */
  0,        /* 0x90: Ctrl-Keypad + */
  "\e[82~", /* 0x91: Ctrl-Down Arrow */
  "\e[83~", /* 0x92: Ctrl-Insert */
  "\e[84~", /* 0x93: Ctrl-Delete */
  0,        /* 0x94 */
  0,        /* 0x95: Ctrl-Keypad / */
  0,        /* 0x96: Ctrl-Keypad * */
  "\e[93~", /* 0x97: Alt-Home */
  "\e[89~", /* 0x98: Alt-Up Arrow */
  "\e[97~", /* 0x99: Alt-Page Up */
  0,        /* 0x9a */
  "\e[92~", /* 0x9b: Alt-Left Arrow */
  0,        /* 0x9c */
  "\e[91~", /* 0x9d: Alt-Right Arrow */
  0,        /* 0x9e */
  "\e[94~", /* 0x9f: Alt-End */
  "\e[90~", /* 0xa0: Alt-Down Arrow */
  "\e[98~", /* 0xa1: Alt-Page Down */
  "\e[95~", /* 0xa2: Alt-Insert */
  "\e[96~", /* 0xa3: Alt-Delete */
            /* 0xa4: Alt-Keypad / */
            /* 0xa5: Alt-Tab */
            /* 0xa6: Alt-Enter */
};

const unsigned char *
__get_extended_key_string(int xkey_code)
{
  int idx;

  /* Strip flags added by getxkey.  */
  xkey_code &= 0xff;

  if (xkey_code < MIN_EXT_SCAN_CODE)
    return NULL;

  idx = xkey_code - MIN_EXT_SCAN_CODE;
  if (idx > ARRAY_SIZEOF(ecma48_encoding_table))
    return NULL;

  return ecma48_encoding_table[idx];
}

