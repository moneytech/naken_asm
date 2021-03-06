/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: http://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2010-2017 by Michael Kohn
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "table/arc.h"

struct _table_arc table_arc[] =
{
  { "abs",    0x266f7009, 0xffff703f, OP_0_U6,  F_F },
  { "abs",    0x262f7f89, 0xffff7fff, OP_0_LIMM,F_F },
  { "abs",    0x262f7009, 0xffff703f, OP_0_C,   F_F },
  { "abs",    0x206f0009, 0xf8ff003f, OP_B_U6,  F_F },
  { "abs",    0x202f0f89, 0xf8ff0fff, OP_B_LIMM, F_F },
  { "abs",    0x202f0009, 0xf8ff003f, OP_B_C,    F_F },
  { "abss",   0x2e6f7005, 0xffff703f, OP_0_U6,   F_F },
  { "abss",   0x2e2f7f85, 0xffff7fff, OP_0_LIMM, F_F },
  { "abss",   0x2e2f7005, 0xffff703f, OP_0_C,    F_F },
  { "abss",   0x286f0005, 0xf8ff003f, OP_B_U6,   F_F },
  { "abss",   0x282f0f85, 0xf8ff0fff, OP_B_LIMM, F_F },
  { "abss",   0x282f0005, 0xf8ff003f, OP_B_C,    F_F },
  { "abssw",  0x2e6f7004, 0xffff703f, OP_0_U6,   F_F },
  { "abssw",  0x2e2f7f84, 0xffff7fff, OP_0_LIMM, F_F },
  { "abssw",  0x2e2f7004, 0xffff703f, OP_0_C,    F_F },
  { "abssw",  0x286f0004, 0xf8ff003f, OP_B_U6,   F_F },
  { "abssw",  0x2e2f7f84, 0xffff7fff, OP_B_LIMM, F_F },
  { "abssw",  0x282f0004, 0xf8ff003f, OP_B_C,    F_F },
  { "adc",    0x2041003e, 0xf8ff003f, OP_0_B_U6,    F_F },
  { "adc",    0x20010fbe, 0xf8ff0fff, OP_0_B_LIMM,  F_F },
  { "adc",    0x26c17000, 0xffff7020, OP_0_LIMM_C,  F_F | F_CC },
  { "adc",    0x2001003e, 0xf8ff003f, OP_0_B_C,     F_F },
  { "adc",    0x20410000, 0xf8ff0000, OP_A_B_U6,    F_F },
  { "adc",    0x20810000, 0xf8ff0000, OP_B_B_S12,   F_F },
  { "adc",    0x20c10020, 0xf8ff0020, OP_B_B_U6,    F_F | F_CC },
  { "adc",    0x26017000, 0xffff7000, OP_A_LIMM_C,  F_F },
  { "adc",    0x20010f80, 0xf8ff0fc0, OP_A_B_LIMM,  F_F },
  { "adc",    0x20c10f80, 0xf8ff0fe0, OP_B_B_LIMM,  F_F | F_CC },
  { "adc",    0x20c10000, 0xf8ff0020, OP_B_B_C,     F_F | F_CC },
  { "adc",    0x20010000, 0xf8ff0000, OP_A_B_C,     F_F },
};

struct _table_arc16 table_arc16[] =
{
  { "abs_s",  0x7811, 0xf81f, OP_B_C },
};

