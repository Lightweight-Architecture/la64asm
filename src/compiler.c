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

#include <la64asm/compiler.h>
#include <la64asm/diag.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <lautils/parser.h>
#include <la64asm/opcode.h>
#include <la64asm/register.h>

#include <lautils/bitwalker.h>

bool la64_compiler_lowcodeline(compiler_line_t *cl)
{
    /* accessing compiler invocation */
    compiler_invocation_t *ci = cl->ci;

    /* parameter count check */
    if(cl->token_cnt <= 0)
    {
        printf("[!] insufficient parameters\n");
    }
    else if(cl->token_cnt > 32)
    {
        printf("[!] too many parameters (32 maximum)\n");
    }

    /* initilize bitwalker */
    bitwalker_t bw;
    bitwalker_init(&bw, &(ci->image[ci->image_addr]), 512, BW_LITTLE_ENDIAN);

    /* getting opcode entry if it exists */
    opcode_entry_t *opce = opcode_from_string(cl->token[0].str);

    if(opce == NULL)
    {
        printf("[!] illegal opcode: %s\n", cl->token[0].str);
        exit(1);
    }
    else
    {
        /* setting opcode from entry */
        bitwalker_write(&bw, opce->opcode, 8);

        switch(opce->opcode)
        {
            case LA64_OPCODE_HLT:
            case LA64_OPCODE_NOP:
            case LA64_OPCODE_RET:
                goto skip_parse;
            default:
                break;
        }
    }

    /* parse parameters */
    for(uint64_t i = 1; i < cl->token_cnt; i++)
    {
        /* parsing value */
        parser_return_t pr = parse_value_from_string(cl->token[i].str);

        /* checking for intermediates */
        if(pr.type != laParserValueTypeString)
        {
            /* now we gonna have to check how large this is ;w; */
            if(pr.value <= 0xFF)
            {
                bitwalker_write(&bw, LA64_PARAMETER_CODING_IMM8, 3);
                bitwalker_write(&bw, pr.value, 8);
            }
            else if(pr.value <= 0xFFFF)
            {
                bitwalker_write(&bw, LA64_PARAMETER_CODING_IMM16, 3);
                bitwalker_write(&bw, pr.value, 16);
            }
            else if(pr.value <= 0xFFFFFFFF)
            {
                bitwalker_write(&bw, LA64_PARAMETER_CODING_IMM32, 3);
                bitwalker_write(&bw, pr.value, 32);
            }
            else if(pr.value <= 0xFFFFFFFFFFFFFFFF)
            {
                bitwalker_write(&bw, LA64_PARAMETER_CODING_IMM64, 3);
                bitwalker_write(&bw, pr.value, 64);
            }

            continue;
        }

        /* checking for register */
        register_entry_t *reg = register_from_string(cl->token[i].str);

        if(reg != NULL)
        {
            bitwalker_write(&bw, LA64_PARAMETER_CODING_REG, 3);
            bitwalker_write(&bw, reg->reg, 5);
            continue;
        }
        
        /* set mode to 64bit lmfao */
        bitwalker_write(&bw, LA64_PARAMETER_CODING_IMM64, 3);

        /* it must be a label and therefore a entry in the new relocation table ;) */
        /* checking label type in question */
        char *label = NULL;

        /* checking for local label */
        if(cl->token[i].str[0] == '.')
        {
            asprintf(&label, "%s%s", ci->label_scope, cl->token[i].str);
        }
        else
        {
            label = strdup(cl->token[i].str);
        }
        
        ci->rtlb[ci->rtlb_cnt].name = label;
        ci->rtlb[ci->rtlb_cnt].bw = bw;
        ci->rtlb[ci->rtlb_cnt++].ctlink = &(cl->token[i]);

        /* skip the 64bit for now */
        bitwalker_skip(&bw, 64);
    }

    bitwalker_write(&bw, LA64_PARAMETER_CODING_INSTR_END, 3);

skip_parse:

    ci->image_addr += bitwalker_bytes_used(&bw);

    return 0;
}

void la64_compiler_lowlevel(compiler_invocation_t *ci)
{
    /* iterate through each token */
    for(uint64_t i = 0; i < ci->line_cnt; i++)
    {
        /* checking for label */
        if(ci->line[i].type == COMPILER_LINE_TYPE_LABEL ||
           ci->line[i].type == COMPILER_LINE_TYPE_LABEL_IN_SCOPE)
        {
            /* insert into labels */
            code_token_label_append(&(ci->line[i].token[0]));
        }
        else if(ci->line[i].type == COMPILER_LINE_TYPE_ASM)
        {
            la64_compiler_lowcodeline(&(ci->line[i]));
        }
    }
    
    /* append binary end label */
    ci->label[ci->label_cnt].addr = ci->image_addr;
    ci->label[ci->label_cnt++].name = strdup("__la64_exec_img_end");

    /* now handling relocations */
    for(uint64_t i = 0; i < ci->rtlb_cnt; i++)
    {
        /* lookup label */
        uint64_t addr = label_lookup(ci, ci->rtlb[i].name);

        /* sanity checking address */
        if(addr == COMPILER_LABEL_NOT_FOUND)
        {
            diag_error(ci->rtlb[i].ctlink, "label \"%s\" not found\n", ci->rtlb[i].name);
        }

        /* using da bitwalker to fixup address */
        bitwalker_write(&(ci->rtlb[i].bw), addr, 64);
    }
}
