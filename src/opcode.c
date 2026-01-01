/*
 * MIT License
 *
 * Copyright (c) 2024 cr4zyengineer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <la64asm/opcode.h>
#include <stdlib.h>
#include <string.h>

opcode_entry_t opcode_table[LA64_OPCODE_MAX + 1] = {
    /* core operations */
    { .name = "hlt", .opcode = LA64_OPCODE_HLT },
    { .name = "nop", .opcode = LA64_OPCODE_NOP },

    /* data operations */
    { .name = "mov", .opcode = LA64_OPCODE_MOV },
    { .name = "swp", .opcode = LA64_OPCODE_SWP },
    { .name = "swpz", .opcode = LA64_OPCODE_SWPZ },
    { .name = "push", .opcode = LA64_OPCODE_PUSH },
    { .name = "pop", .opcode = LA64_OPCODE_POP },
    { .name = "ldb", .opcode = LA64_OPCODE_LDB },
    { .name = "ldw", .opcode = LA64_OPCODE_LDW },
    { .name = "ldd", .opcode = LA64_OPCODE_LDD },
    { .name = "ldq", .opcode = LA64_OPCODE_LDQ },
    { .name = "stb", .opcode = LA64_OPCODE_STB },
    { .name = "stw", .opcode = LA64_OPCODE_STW },
    { .name = "std", .opcode = LA64_OPCODE_STD },
    { .name = "stq", .opcode = LA64_OPCODE_STQ },

    /* io operations */
    { .name = "in", .opcode = LA64_OPCODE_IN },
    { .name = "out", .opcode = LA64_OPCODE_OUT },

    /* alu operations */
    { .name = "add", .opcode = LA64_OPCODE_ADD },
    { .name = "sub", .opcode = LA64_OPCODE_SUB },
    { .name = "mul", .opcode = LA64_OPCODE_MUL },
    { .name = "div", .opcode = LA64_OPCODE_DIV },
    { .name = "idiv", .opcode = LA64_OPCODE_IDIV },
    { .name = "mod", .opcode = LA64_OPCODE_MOD },
    { .name = "inc", .opcode = LA64_OPCODE_INC },
    { .name = "dec", .opcode = LA64_OPCODE_DEC },
    { .name = "not", .opcode = LA64_OPCODE_NOT },
    { .name = "and", .opcode = LA64_OPCODE_AND },
    { .name = "or", .opcode = LA64_OPCODE_OR },
    { .name = "xor", .opcode = LA64_OPCODE_XOR },
    { .name = "shr", .opcode = LA64_OPCODE_SHR },
    { .name = "shl", .opcode = LA64_OPCODE_SHL },
    { .name = "ror", .opcode = LA64_OPCODE_ROR },
    { .name = "rol", .opcode = LA64_OPCODE_ROL },

    /* contol flow operations */
    { .name = "jmp", .opcode = LA64_OPCODE_JMP },
    { .name = "cmp", .opcode = LA64_OPCODE_CMP },
    { .name = "je", .opcode = LA64_OPCODE_JE },
    { .name = "jne", .opcode = LA64_OPCODE_JNE },
    { .name = "jlt", .opcode = LA64_OPCODE_JLT },
    { .name = "jgt", .opcode = LA64_OPCODE_JGT },
    { .name = "jle", .opcode = LA64_OPCODE_JLE },
    { .name = "jge", .opcode = LA64_OPCODE_JGE },
    { .name = "jz", .opcode = LA64_OPCODE_JZ },
    { .name = "jnz", .opcode = LA64_OPCODE_JNZ },
    { .name = "bl", .opcode = LA64_OPCODE_BL },
    { .name = "ret", .opcode = LA64_OPCODE_RET },
};

opcode_entry_t *opcode_from_string(const char *name)
{
    /* null pointer check */
    if(name == NULL)
    {
        return NULL;
    }

    /* iterating through table */
    for(unsigned char opcode = 0x00; opcode < LA64_OPCODE_MAX + 1; opcode++)
    {
        /* check if opcode name matches */
        if(strcmp(opcode_table[opcode].name, name) == 0)
        {
            return &opcode_table[opcode];
        }
    }

    /* shouldnt happen if code is correct */
    return NULL;
}