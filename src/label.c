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
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <la64asm/label.h>
#include <unistd.h>

bool code_label_exists(compiler_invocation_t *ci,
                       const char *name)
{
    /* null pointer check */
    if(ci == NULL || name == NULL)
    {
        return false;
    }

    /* iterate through all labels */
    for(int i = 0; i < ci->label_cnt; i++)
    {
        if(strcmp(ci->label[i].name, name) == 0)
        {
            return true;
        }
    }

    /* it doesnt exist */
    return false;
}

void code_token_label(compiler_invocation_t *ci)
{
    /* counting labels caught at token parsing */
    ci->label_cnt = 1;
    for(int i = 0; i < ci->token_cnt; i++)
    {
        if(ci->token[i].type == COMPILER_TOKEN_TYPE_LABEL ||
           ci->token[i].type == COMPILER_TOKEN_TYPE_LABEL_IN_SCOPE ||
           ci->token[i].type == COMPILER_TOKEN_TYPE_SECTION_DATA)
        {
            (ci->label_cnt)++;
        }
    }

    /* allocating memory for those */
    ci->label = calloc(ci->label_cnt, sizeof(compiler_label_t));

    /* reset label count for compiler */
    ci->label_cnt = 0;
}

void code_token_label_append(compiler_invocation_t *ci,
                             compiler_token_t *ct)
{
    /* null pointer check */
    if(ci == NULL || ci->label == NULL || ct == NULL)
    {
        return;
    }

    /* assign address to label */
    ci->label[ci->label_cnt].addr = ci->image_addr;

    /* copying label name */
    size_t size = strlen(ct->token);
    char *name = malloc(size);
    memcpy(name, ct->token, size - 1);
    name[size - 1] = '\0';

    /* checking if its in scope */
    if(ct->type == COMPILER_TOKEN_TYPE_LABEL_IN_SCOPE)
    {
        /* null poiner checking scope */
        if(ci->label_scope == NULL)
        {
            printf("[!] no scope for label in scope\n");
            exit(1);
        }

        /* copy name temporairly  */
        char *tmp_name = strdup(name);

        /* adjust size */
        size += strlen(ci->label_scope);

        /* reallocate buffer */
        name = realloc(name, (size) + 1);

        /* recopy */
        sprintf(name, "%s%s", ci->label_scope, ct->token);
        name[size - 1] = '\0';
    }
    else
    {
        /* set it as scope */
        ci->label_scope = name;
    }

    /* checking for duplicated labels */
    if(code_label_exists(ci, name))
    {
        printf("[!] duplicate label: %s\n", name);
        exit(1);
    }

    ci->label[ci->label_cnt++].name = name;
}

uint64_t label_lookup(compiler_invocation_t *ci,
                      const char *name)
{
    /* iterating through all labels */
    for(int i = 0; i < ci->label_cnt; i++)
    {
        /* checking if request name matches */
        if(strcmp(ci->label[i].name, name) == 0)
        {
            /* returning label address*/
            return ci->label[i].addr;
        }
    }

    return COMPILER_LABEL_NOT_FOUND;
}

void code_token_label_insert_start(compiler_invocation_t *ci)
{
    /* finding start label */
    uint64_t addr = label_lookup(ci, "_start");
    if(addr == COMPILER_LABEL_NOT_FOUND)
    {
        exit(1);
    }

    /* writing start address into the start of the image */
    bitwalker_t bw;
    bitwalker_init(&bw, ci->image, 8, BW_LITTLE_ENDIAN);
    bitwalker_write(&bw, addr, 64);
}