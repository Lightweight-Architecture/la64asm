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

#ifndef COMPILER_TYPE_H
#define COMPILER_TYPE_H

#include <stdlib.h>
#include <stdint.h>

#include <lautils/bitwalker.h>

#define COMPILER_TOKEN_TYPE_ASM              0b000
#define COMPILER_TOKEN_TYPE_LABEL            0b001
#define COMPILER_TOKEN_TYPE_LABEL_IN_SCOPE   0b010
#define COMPILER_TOKEN_TYPE_SECTION          0b011
#define COMPILER_TOKEN_TYPE_SECTION_DATA     0b100
#define COMPILER_TOKEN_TYPE_SECTION_MACRO    0b101
#define COMPILER_TOKEN_TYPE_SECTION_MACROEND 0b110
#define COMPILER_TOKEN_TYPE_SECTION_MACRODEF 0b111

typedef unsigned char compiler_token_type_t;

typedef struct {
    compiler_token_type_t type;             /* type of token */
    char *token;                            /* token it self */
    char **subtoken;                        /* subtokens */
    uint64_t subtoken_cnt;                  /* count of subtokens */
} compiler_token_t;

typedef struct {
    char *name;                             /* name of resolved label */
    uint64_t addr;                          /* address of resolved label */
} compiler_label_t;

typedef struct {
    char *name;                             /* unknown label looking for address */
    bitwalker_t bw;                         /* bitwalker state of when it was looked for (always 64bit skipped) */
} reloc_table_entry;

typedef struct {
    char *code;                             /* raw code */
    compiler_token_t *token;                /* token array */
    uint64_t token_cnt;                     /* count of tokens */
    char *label_scope;                      /* current resolved label scope */
    compiler_label_t *label;                /* label array */
    uint64_t label_cnt;                     /* count of labels */
    uint64_t label_cnt_sec;                 /* count of labels */
    reloc_table_entry rtlb[0xFFFF];         /* relocation table */
    uint64_t rtlb_cnt;                      /* count of relocation table entries */
    uint8_t image[0xFFFFFF];                /* replace with better technique that is more incremental */
    uint64_t image_addr;                    /* current address */
    uint64_t image_text_start_addr;         /* the start address of the text region */
} compiler_invocation_t;

#endif /* COMPILER_TYPE_H */
