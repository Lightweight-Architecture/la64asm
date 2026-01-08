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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <la64asm/section.h>
#include <lautils/parser.h>
#include <lautils/bitwalker.h>
#include <la64asm/code.h>

void code_token_section(compiler_invocation_t *ci)
{
    /* iterating for section token type */
    for(unsigned long i = 0; i < ci->token_cnt; i++)
    {
        if(ci->token[i].type == COMPILER_TOKEN_TYPE_SECTION)
        {
            if(strcmp(ci->token[i].subtoken[1], ".data") == 0)
            {
                /* iterating till section data is over */
                i++;
                for(; i < ci->token_cnt && ci->token[i].type == COMPILER_TOKEN_TYPE_SECTION_DATA; i++)
                {
                    /* checking count */
                    if(ci->token[i].subtoken_cnt < 3)
                    {
                        printf("[!] not enough tokens for section data in .data\n");
                        exit(1);
                    }

                    /* inserting address as label */
                    ci->label[ci->label_cnt].name = strdup(ci->token[i].subtoken[0]);
                    ci->label[ci->label_cnt++].addr = ci->image_addr;

                    /* checking if its known */
                    int dbs = 8;
                    if(strcmp(ci->token[i].subtoken[1], "dw") == 0)
                    {
                        dbs = 16;
                    }
                    else if(strcmp(ci->token[i].subtoken[1], "dd") == 0)
                    {
                        dbs = 32;
                    }
                    else if(strcmp(ci->token[i].subtoken[1], "dq") == 0)
                    {
                        dbs = 64;
                    }
                    else if(strcmp(ci->token[i].subtoken[1], "db") != 0)
                    {
                        printf("[!] %s is not a valid data type for .data sections\n", ci->token[i].subtoken[1]);
                        exit(1);
                    }

                    /* iterating through the chain */
                    for(unsigned long a = 2; a < ci->token[i].subtoken_cnt; a++)
                    {
                        /* using low level type parser */
                        parser_return_t pr = parse_value_from_string(ci->token[i].subtoken[a]);

                        /* checking type */
                        if(pr.type == laParserValueTypeBuffer)
                        {
                            /* its a buffer so we copy the buffer into section */
                            char *buffer = (char*)pr.value;
                            for(unsigned short j = 0; j < pr.len; j++)
                            {
                                ci->image[ci->image_addr + j] = (unsigned char)buffer[j];
                            }
                            ci->image_addr += pr.len;
                        }
                        else
                        {
                            bitwalker_t bw;

                            /* storing value */
                            bitwalker_init(&bw, &(ci->image[ci->image_addr]), dbs / 8, BW_LITTLE_ENDIAN);
                            bitwalker_write(&bw, pr.value, dbs);
                            ci->image_addr += bitwalker_bytes_used(&bw);
                        }
                    }
                }
                i--;
            }
            else if(strcmp(ci->token[i].subtoken[1], ".bss") == 0)
            {
                /* finding variable type */
                i++;
                for(; i < ci->token_cnt && ci->token[i].type == COMPILER_TOKEN_TYPE_SECTION_DATA; i++)
                {
                    /* checking count */
                    if(ci->token[i].subtoken_cnt < 2)
                    {
                        printf("[!] not enough tokens for section data in .bss\n");
                        exit(1);
                    }

                    /* insert label into label array */
                    ci->label[ci->label_cnt].name = strdup(ci->token[i].subtoken[0]);
                    ci->label[ci->label_cnt++].addr = ci->image_addr;

                    /* offset image address by value */
                    parser_return_t pr = parse_value_from_string(ci->token[i].subtoken[1]);

                    /* checking if the type makes sense */

                    ci->image_addr += pr.value;
                }
                i--;
            }
        }
    }

    ci->image_text_start_addr = ci->image_addr;
}