/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: http://www.mikekohn.net/
 * License: GPL
 *
 * Copyright 2010-2015 by Michael Kohn, Joe Davisson
 *
 * 65816 by Joe Davisson
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "asm/common.h"
#include "asm/65816.h"
#include "common/assembler.h"
#include "disasm/65816.h"
#include "common/tokens.h"
#include "common/eval_expression.h"

#include "table/65816.h"

#define GET_TOKEN() (token_type = tokens_get(asm_context, token, TOKENLEN))

static int get_num(struct _asm_context *asm_context,
                   char *token, int *token_type,
                   int *num, int *size)
{
  char modifier = 0;

  if(IS_TOKEN(token, '<'))
    modifier = '<';
  else if(IS_TOKEN(token, '>'))
    modifier = '>';
  else
    tokens_push(asm_context, token, *token_type);

  if(eval_expression(asm_context, num) != 0)
  {
    if(asm_context->pass == 1)
      eat_operand(asm_context);
    else
      return -1;
  }

  if(modifier == '<')
  {
    *num &= 0xFF;
    *size = 8;
  }
  else if(modifier == '>')
  {
    *num >>= 8;
    *num &= 0xFF;
    *size = 8;
  }

  return 0;
}

static int get_address(struct _asm_context *asm_context,
                       char *token, int *token_type,
                       int *num, int *size)
{
  char modifier = 0;

  if(IS_TOKEN(token, '<'))
    modifier = '<';
  else if(IS_TOKEN(token, '!'))
    modifier = '!';
  else if(IS_TOKEN(token, '>'))
    modifier = '>';
  else
    tokens_push(asm_context, token, *token_type);

  if(eval_expression(asm_context, num) != 0)
  {
    if(asm_context->pass == 1)
      eat_operand(asm_context);
    else
      return -1;
  }

  if(modifier == '<')
  {
    // force direct-page mode
    *num &= 0xFF;
    *size = 8;
  }
  else if(modifier == '!')
  {
    // force absolute mode
    *num &= 0xFFFF;
    *size = 16;
  }
  else if(modifier == '>')
  {
    // force absolute-long mode
    *num &= 0xFFFFFF;
    *size = 24;
  }

  return 0;
}

extern struct _table_65816 table_65816[];
extern struct _table_65816_opcodes table_65816_opcodes[];

// bytes for each addressing mode
static int op_bytes[] =
{
  1, 2, 3, 2, 3, 4, 2, 2, 3, 3, 4, 2,
  2, 3, 3, 2, 3, 2, 2, 3, 2, 3, 2, 2
};

int parse_directive_65816(struct _asm_context *asm_context,
                          const char *directive)
{
  return 1;
}

int parse_instruction_65816(struct _asm_context *asm_context, char *instr)
{
  char token[TOKENLEN];
  char instr_case[TOKENLEN];
  char temp[256];
  int token_type;
  int opcode;
  int op;
  int instr_enum;
  int num;
  int size;
  int bytes;
  int i;
  int src, dst;
  int label;

  // reset label flag
  label = 0;

  // make lowercase
  lower_copy(instr_case, instr);

  // get instruction from string
  instr_enum = -1;

  if(strcmp(instr_case, "jml") == 0)
  {
    instr_enum = M65816_JMP;
    op = OP_NONE;
    size = 24;  
  }
  else if(strcmp(instr_case, "jsl") == 0)
  {
    instr_enum = M65816_JSR;
    op = OP_NONE;
    size = 24;
  }
  else
  {
    for(i = 0; i < 92; i++)
    {
      if(strcmp(instr_case, table_65816[i].name) == 0)
      {
        instr_enum = i;
        break;
      }
    }

    // no matching instruction
    if(instr_enum == -1)
    {
     print_error_unexp(token, asm_context);
     return -1;
    }

    // get default addressing mode
    op = table_65816[instr_enum].op;

    size = 0;
  }

  // parse
  while(1)
  {
    if(GET_TOKEN() == TOKEN_EOL)
      break;

    if(IS_TOKEN(token, '.'))
    {
      if(GET_TOKEN() == TOKEN_EOL)
        break;

      if(IS_TOKEN(token, 'b') || IS_TOKEN(token, 'B'))
      {
        size = 8;
      }
      else if(IS_TOKEN(token, 'w') || IS_TOKEN(token, 'W'))
      {
        size = 16;
      }
      else if(IS_TOKEN(token, 'l') || IS_TOKEN(token, 'L'))
      {
        size = 24;
      }
      else
      {
        print_error_unexp(token, asm_context);
        return -1;
      }
 
      if(GET_TOKEN() == TOKEN_EOL)
        break;
    }

    if(op == OP_RELATIVE)
    {
      if(IS_TOKEN(token, '#'))
      {
        if(GET_TOKEN() == TOKEN_EOL)
          break;

        if(get_num(asm_context, token, &token_type, &num, &size) == -1)
          return -1;

        if(num < -128 || num > 0xFF)
        {
          print_error("8-bit constant out of range.", asm_context);
          return -1;
        }

        num = (uint8_t)num;
      }
      else
      {
        tokens_push(asm_context, token, token_type);

        if(GET_TOKEN() == TOKEN_EOL)
          break;

        if(get_num(asm_context, token, &token_type, &num, &size) == -1)
          return -1;

        if(asm_context->pass == 2)
        {
          // calculate branch offset, need to add 2 to current
          // address, since thats where the program counter would be
          num -= (asm_context->address + 2);

          if(num < -128 || num > 127)
          {
            print_error("Relative branch out of range", asm_context);
            return -1;
          }

          num = (uint8_t)num;
        }
      }
    }
    else if(op == OP_RELATIVE_LONG)
    {
      tokens_push(asm_context, token, token_type);

      if(GET_TOKEN() == TOKEN_EOL)
        break;

      if(get_num(asm_context, token, &token_type, &num, &size) == -1)
        return -1;

      if(asm_context->pass == 2)
      {
        // calculate branch offset, need to add 3 to current
        // address, since thats where the program counter would be
        num -= (asm_context->address + 3);

        if(num < -32768 || num > 32767)
        {
          print_error("Relative long branch out of range", asm_context);
          return -1;
        }

        num = (uint16_t)num;
      }
    }
    else if(op == OP_BLOCK_MOVE)
    {
      tokens_push(asm_context, token, token_type);

      if(GET_TOKEN() == TOKEN_EOL)
        break;

      if(get_address(asm_context, token, &token_type, &num, &size) == -1)
        return -1;

      src = num;

      if(src < 0 || src > 0xFF)
      {
        print_error("Block move source address out of range.", asm_context);
        return -1;
      }

      if(GET_TOKEN() == TOKEN_EOL)
        break;

      if(IS_NOT_TOKEN(token, ','))
      {
        print_error_unexp(token, asm_context);
        return -1;
      }

      if(GET_TOKEN() == TOKEN_EOL)
        break;

      tokens_push(asm_context, token, token_type);

      if(GET_TOKEN() == TOKEN_EOL)
        break;

      if(get_address(asm_context, token, &token_type, &num, &size) == -1)
        return -1;

      dst = num;

      if(dst < 0 || dst > 0xFF)
      {
        print_error("Block move destination address out of range.", asm_context);
        return -1;
      }

      num = src + (dst << 8);
    }
    else
    {
      if(IS_TOKEN(token, '#'))
      {
        if(GET_TOKEN() == TOKEN_EOL)
          break;

        if(get_num(asm_context, token, &token_type, &num, &size) == -1)
          return -1;

        if(op == OP_IMMEDIATE8)
          size = 8;
        else
          op = OP_IMMEDIATE16;

        if(size == 8)
        {
          if(num < -128 || num > 0xFF)
          {
            print_error("8-bit constant out of range.", asm_context);
            return -1;
          }

          num = (uint8_t)num;
        }
        else if(size == 16)
        {
          if(num < -32768 || num > 0xFFFF)
          {
            print_error("16-bit constant out of range.", asm_context);
            return -1;
          }

          num = (uint16_t)num;
        }
        else if(size == 24)
        {
          print_error("Cannot force long value in immediate mode.", asm_context);
          return -1;
        }
      }
      else if(IS_TOKEN(token, '('))
      {
        if(GET_TOKEN() == TOKEN_EOL)
          break;

        if(get_address(asm_context, token, &token_type, &num, &size) == -1)
          return -1;

        if(GET_TOKEN() == TOKEN_EOL)
          break;

        if(IS_TOKEN(token, ','))
        {
          if(GET_TOKEN() == TOKEN_EOL)
            break;

          if(IS_TOKEN(token, 'x') || IS_TOKEN(token, 'X'))
          {
            if(GET_TOKEN() == TOKEN_EOL)
              break;

            if(IS_TOKEN(token, ')'))
            {
              if(num < 0 || num > 0xFFFF)
              {
                print_error("Address out of range.", asm_context);
                return -1;
              }

              if(num > 0xFF)
                op = OP_X_INDIRECT16;
              else
                op = OP_X_INDIRECT8;
            }
            else
            {
              print_error_unexp(token, asm_context);
              return -1;
            }
          }
          else if(IS_TOKEN(token, 's') || IS_TOKEN(token, 'S'))
          {
            op = OP_SP_INDIRECT_Y;

            if(GET_TOKEN() == TOKEN_EOL)
              break;

            if(IS_NOT_TOKEN(token, ')'))
            {
              print_error_unexp(token, asm_context);
              return -1;
            }

            if(GET_TOKEN() == TOKEN_EOL)
              break;

            if(IS_NOT_TOKEN(token, ','))
            {
              print_error_unexp(token, asm_context);
              return -1;
            }

            if(GET_TOKEN() == TOKEN_EOL)
              break;

            if(IS_NOT_TOKEN(token, 'y') && IS_NOT_TOKEN(token, 'Y'))
            {
              print_error_unexp(token, asm_context);
              return -1;
            }
          }
          else
          {
            print_error_unexp(token, asm_context);
            return -1;
          }
        }
        else if(IS_TOKEN(token, ')'))
        {
          op = OP_INDIRECT8;

          if(GET_TOKEN() == TOKEN_EOL)
            break;

          if(IS_TOKEN(token, ','))
          {
            if(GET_TOKEN() == TOKEN_EOL)
              break;

            if(IS_TOKEN(token, 'y') || IS_TOKEN(token, 'Y'))
            {
              op = OP_INDIRECT8_Y;
            }
            else
            {
              print_error_unexp(token, asm_context);
              return -1;
            }
          }
          else
          {
            print_error_unexp(token, asm_context);
            return -1;
          }
        }
        else
        {
          print_error_unexp(token, asm_context);
          return -1;
        }
      }
      else if(IS_TOKEN(token, '['))
      {
        if(GET_TOKEN() == TOKEN_EOL)
          break;

        if(get_address(asm_context, token, &token_type, &num, &size) == -1)
          return -1;

        if(GET_TOKEN() == TOKEN_EOL)
          break;

        if(IS_TOKEN(token, ']'))
        {
          op = OP_INDIRECT8_LONG;

          if(GET_TOKEN() == TOKEN_EOL)
            break;

          if(IS_TOKEN(token, ','))
          {
            if(GET_TOKEN() == TOKEN_EOL)
              break;

            if(IS_TOKEN(token, 'y') || IS_TOKEN(token, 'Y'))
            {
              op = OP_INDIRECT8_Y_LONG;
            }
            else
            {
              print_error_unexp(token, asm_context);
              return -1;
            }
          }
          else
          {
            print_error_unexp(token, asm_context);
            return -1;
          }
        }
      }
      else
      {
        tokens_push(asm_context, token, token_type);

        if(GET_TOKEN() == TOKEN_EOL)
          break;

        if(get_address(asm_context, token, &token_type, &num, &size) == -1)
          return -1;

        // if all zeros then this is a forward-label
        if(asm_context->pass == 2)
        {
          if((memory_read(asm_context, asm_context->address + 1) == 0) &&
             (memory_read(asm_context, asm_context->address + 2) == 0) &&
             (memory_read(asm_context, asm_context->address + 3) == 0))
          {
            op = OP_ADDRESS24;
            label = 1;
          }
        }

        if(num < 0 || num > 0xFFFFFF)
        {
          print_error("Address out of range.", asm_context);
          return -1;
        }

        if(op != OP_ADDRESS24)
        {
          op = OP_ADDRESS8;

          if(num > 0xFF)
            op = OP_ADDRESS16;

          if(num > 0xFFFF)
            op = OP_ADDRESS24;
        }

        if(asm_context->pass == 1 && num == 0)
          op = OP_ADDRESS24;

        if(size == 8)
        {
          if(num > 0xFF)
          {
            print_error("Direct-page address out of range.", asm_context);
            return -1;
          }

          op = OP_ADDRESS8;
        }
        else if(size == 16)
        {
          if(num > 0xFFFF)
          {
            print_error("Absolute address out of range.", asm_context);
            return -1;
          }

          op = OP_ADDRESS16;
        }
        else if(size == 24)
        {
          if(num > 0xFFFFFF)
          {
            print_error("Absolute long address out of range.", asm_context);
            return -1;
          }

          op = OP_ADDRESS24;
        }

        if(GET_TOKEN() == TOKEN_EOL)
          break;

        if(IS_TOKEN(token, ','))
        {
          if(GET_TOKEN() == TOKEN_EOL)
            break;

          if(IS_TOKEN(token, 'x') || IS_TOKEN(token, 'X'))
          {
            op = OP_INDEXED8_X;

            if(size == 16 || num > 0xFF)
              op = OP_INDEXED16_X;

            if(size == 24 || num > 0xFFFF)
              op = OP_INDEXED24_X;

            if(num > 0xFFFFFF)
            {
              print_error("Address out of range.", asm_context);
              print_error_unexp(token, asm_context);
              return -1;
            }
          }
          else if(IS_TOKEN(token, 'y') || IS_TOKEN(token, 'Y'))
          {
            op = OP_INDEXED8_Y;

            if(size == 16 || num > 0xFF)
              op = OP_INDEXED16_Y;

            if(size == 24 || num > 0xFFFF)
            {
              print_error("Absolute long not supported for Y indexing.",
                          asm_context);
              print_error_unexp(token, asm_context);
              return -1;
            }
          }
          else if(IS_TOKEN(token, 's'))
          {
            op = OP_SP_RELATIVE;

            if(num > 0xFF)
            {
              print_error("Address out of range.", asm_context);
              print_error_unexp(token, asm_context);
              return -1;
            }
          }
          else
          {
            print_error_unexp(token, asm_context);
            return -1;
          }
        }
        else
        {
          print_error_unexp(token, asm_context);
          return -1;
        }
      }
    }
  }

  // add a NOP (0xEA) after 16-bit forward-label addresses 
  if(label == 1 && size == 0)
  {
    if(((instr_enum == M65816_JMP) || (instr_enum == M65816_JSR))
       && (num <= 0xFFFF))
    {
      num |= 0xEA0000; 
      // temporarily change addressing mode
      // so the proper instruction can be found
      op = OP_ADDRESS16;
    }
  }

  opcode = -1;

  for(i = 0; i < 256; i++)
  {
    if(table_65816_opcodes[i].instr == instr_enum)
    {
      if(table_65816_opcodes[i].op == op)
      {
        opcode = i;
        break;
      }
      else if(op == OP_ADDRESS8)
      {
        if(table_65816_opcodes[i].op == OP_ADDRESS16)
        {
          op = OP_ADDRESS16;
          opcode = i;
          break;
        }
      }
      else if(op == OP_ADDRESS16)
      {
        if(table_65816_opcodes[i].op == OP_ADDRESS24)
        {
          op = OP_ADDRESS24;
          opcode = i;
          break;
        }
      }
      else if(op == OP_INDIRECT8)
      {
        if(table_65816_opcodes[i].op == OP_INDIRECT16)
        {
          op = OP_INDIRECT16;
          opcode = i;
          break;
        }
      }
      else if(op == OP_INDIRECT8_LONG)
      {
        if(table_65816_opcodes[i].op == OP_INDIRECT16_LONG)
        {
          op = OP_INDIRECT16_LONG;
          opcode = i;
          break;
        }
      }
      else if(op == OP_X_INDIRECT8)
      {
        if(table_65816_opcodes[i].op == OP_X_INDIRECT16)
        {
          op = OP_X_INDIRECT16;
          opcode = i;
          break;
        }
      }
    }
  }

  if(asm_context->pass == 2 && opcode == -1)
  {
    sprintf(temp, "No instruction found for addressing mode %d", op);
    print_error(temp, asm_context);
    return -1;
  }

  // change addressing mode back so that storage size is correct
  if(label == 1)
      op = OP_ADDRESS24;

  if(size == 8 && op == OP_IMMEDIATE16)
    bytes = op_bytes[OP_IMMEDIATE8];
  else
    bytes = op_bytes[op];

  add_bin8(asm_context, opcode & 0xFF, IS_OPCODE);

  if(bytes > 1)
    add_bin8(asm_context, num & 0xFF, IS_OPCODE);

  if(bytes > 2)
    add_bin8(asm_context, (num >> 8) & 0xFF, IS_OPCODE);

  if(bytes > 3)
    add_bin8(asm_context, (num >> 16) & 0xFF, IS_OPCODE);

  return bytes;
}

