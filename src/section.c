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
#include <la64asm/code.h>

static char *trim(char *str)
{
    /* null pointer check */
    if(str == NULL)
    {
        return NULL;
    }

    /* checking if its a space */
    while(isspace((unsigned char)*str))
    {
        str++;
    }

    /* checking if its a null terminator*/
    if(*str == '\0')
    {
        return str;
    }

    /* calculating the end */
    char *end = str + strlen(str) - 1;

    /* checking if its a space */
    while(end > str && isspace((unsigned char)*end))
    {
        end--;
    }

    /* null terminating */
    end[1] = '\0';
    
    return str;
}

static char **parse_csv_quoted(const char *input, unsigned long *count)
{
    /* null pointer check */
    if(input == NULL || count == NULL)
    {
        return NULL;
    }

    /* duplicate input */
    char *token = strdup(input);
    
    unsigned long chain_cnt = 0;
    char *ptr = token;
    int in_quotes = 0;

    /* waiting till input is null terminated */
    while(*ptr)
    {
        if(*ptr == '"' || *ptr == '\'')
        {
            in_quotes = !in_quotes;
        }
        else if(*ptr == ',' && !in_quotes)
        {
            chain_cnt++;
        }
        ptr++;
    }
    chain_cnt++;

    /* allocating chain */
    char **chain = calloc(chain_cnt, sizeof(char*));

    free(token);
    token = strdup(input);
    ptr = token;
    in_quotes = 0;
    char *start = ptr;
    chain_cnt = 0;

    /* second pass */
    while(*ptr)
    {
        if(*ptr == '"' || *ptr == '\'')
        {
            in_quotes = !in_quotes;
        }
        else if(*ptr == ',' && !in_quotes)
        {
            *ptr = '\0';
            chain[chain_cnt++] = strdup(trim(start));
            start = ptr + 1;
        }
        ptr++;
    }
    if(start <= ptr)
    {
        chain[chain_cnt++] = strdup(trim(start));
    }

    free(token);
    *count = chain_cnt;
    return chain;
}

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
                    /* inserting address as label */
                    ci->label_cnt++;
                    ci->label[ci->label_cnt - 1].name = strdup(ci->token[i].subtoken[0]);
                    ci->label[ci->label_cnt - 1].addr = ci->image_addr;

                    /* checking if its known */
                    int is_word = 0;
                    if(strcmp(ci->token[i].subtoken[1], "dw") == 0)
                    {
                        is_word = 1;
                    }
                    else if(strcmp(ci->token[i].subtoken[1], "db") != 0)
                    {
                        printf("[!] %s is not a valid data type for .data sections\n", ci->token[i].subtoken[1]);
                        exit(1);
                    }

                    /* binding token at a certain position by its subtokens */
                    unsigned long chain_cnt = 0;
                    char *chain_str = code_token_bind(&ci->token[i], 2);

                    /* null pointer check */
                    if(chain_str == NULL)
                    {
                        printf("[!] null pointer exception\n");
                        exit(1);
                    }

                    /* getting chain */
                    char **chain = parse_csv_quoted(chain_str, &chain_cnt);

                    /* null pointer check */
                    if(chain == NULL)
                    {
                        printf("[!] null pointer exception\n");
                        exit(1);
                    }

                    /* freeing bound chain string buffer */
                    free(chain_str);

                    /* iterating through the chain */
                    for(unsigned long a = 0; a < chain_cnt; a++)
                    {
                        /* using low level type parser */
                        parser_return_t pr = parse_value_from_string(chain[a]);

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
                            /* storing value */
                            if(is_word)
                            {
                                ci->image[ci->image_addr] = pr.value & 0xFF;
                                ci->image[ci->image_addr + 1] = (pr.value >> 8) & 0xFF;
                                ci->image_addr += 2;
                            }
                            else
                            {
                                ci->image[ci->image_addr] = pr.value;
                                ci->image_addr++;
                            }
                        }
                        free(chain[a]);
                    }
                    free(chain);
                }
                i--;
            }
            else if(strcmp(ci->token[i].subtoken[1], ".bss") == 0)
            {
                /* finding variable type */
                i++;
                for(; i < ci->token_cnt && ci->token[i].type == COMPILER_TOKEN_TYPE_SECTION_DATA; i++)
                {
                    /* insert label into label array */
                    ci->label_cnt++;
                    ci->label[ci->label_cnt - 1].name = strdup(ci->token[i].subtoken[0]);
                    ci->label[ci->label_cnt - 1].addr = ci->image_addr;

                    /* offset image address by value */
                    parser_return_t pr = parse_value_from_string(ci->token[i].subtoken[1]);
                    ci->image_addr += pr.value;
                }
                i--;
            }
        }
    }

    ci->image_text_start_addr = ci->image_addr;
}