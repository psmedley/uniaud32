// SPDX-License-Identifier: GPL-2.0
/* Fixes for Lenovo Ideapad S740, to be included from codec driver */

static const struct hda_verb alc285_ideapad_s740_coefs[] = {
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x10 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0320 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x24 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0041 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x24 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0041 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x007f },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x007f },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x003c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0011 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x003c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0011 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x000c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001a },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x000c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001a },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x000f },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0042 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x000f },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0042 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0010 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0040 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0010 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0040 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0003 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0009 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0003 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0009 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x004c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x004c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001d },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x004e },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001d },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x004e },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001b },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001b },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0019 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0025 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0019 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0025 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0018 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0037 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0018 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0037 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001a },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0040 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001a },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0040 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0016 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0076 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0016 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0076 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0017 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0010 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0017 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0010 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0015 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0015 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0015 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0015 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0007 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0086 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0007 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0086 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0002 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0002 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0002 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0002 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x24 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0042 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x24 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0042 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x007f },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x007f },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x003c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0011 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x003c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0011 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x000c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x002a },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x000c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x002a },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x000f },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0046 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x000f },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0046 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0010 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0044 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0010 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0044 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0003 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0009 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0003 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0009 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x004c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x004c },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001b },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001b },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0019 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0025 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0019 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0025 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0018 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0037 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0018 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0037 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001a },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0040 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x001a },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0040 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0016 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0076 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0016 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0076 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0017 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0010 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0017 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0010 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0015 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0015 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0015 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0015 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0007 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0086 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0007 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0086 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0002 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0002 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0001 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x29 },
{ 0x20, AC_VERB_SET_COEF_INDEX, 0x26 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0002 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0x0000 },
{ 0x20, AC_VERB_SET_PROC_COEF, 0xb020 },
{0}
};

static void alc285_fixup_ideapad_s740_coef(struct hda_codec *codec,
					   const struct hda_fixup *fix,
					   int action)
{
	switch (action) {
	case HDA_FIXUP_ACT_PRE_PROBE:
		snd_hda_add_verbs(codec, alc285_ideapad_s740_coefs);
		break;
	}
}
