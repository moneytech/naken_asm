/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: http://www.mikekohn.net/
 * License: GPL
 *
 * Copyright 2010-2016 by Michael Kohn
 *
 */

#ifndef _TABLE_8051_H
#define _TABLE_8051_H

#include "common/assembler.h"

enum
{
  OP_NONE,
  OP_REG,
  OP_AT_REG,
  OP_A,
  OP_C,
  OP_AB,
  OP_DPTR,
  OP_AT_A_PLUS_DPTR,
  OP_AT_A_PLUS_PC,
  OP_AT_DPTR,
  OP_DATA_16,
  OP_CODE_ADDR,
  OP_BIT_ADDR,
  OP_DATA,
  OP_PAGE,
  OP_RELADDR,
  OP_SLASH_BIT_ADDR,
  OP_IRAM_ADDR,
};

struct _table_8051
{
  char *name;
  char op[3];
  char range;   // range of register or page (r0, r1, r2 etc)
};

extern struct _table_8051 table_8051[];

#endif


