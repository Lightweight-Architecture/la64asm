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
#include <la64asm/compile.h>
#include <la64asm/code.h>
#include <la64asm/label.h>
#include <la64asm/compiler.h>
#include <la64asm/section.h>
#include <la64asm/macro.h>

compiler_invocation_t *compiler_invocation_alloc(void)
{
    compiler_invocation_t *ci = calloc(1, sizeof(compiler_invocation_t));
    ci->image_addr = 8;
    return ci;
}

void compiler_invocation_dealloc(compiler_invocation_t *ci)
{
    /* freeing the code it self */
    free(ci->code);

    /* freeing the token structures */
    for(unsigned long i = 0; i < ci->token_cnt; i++)
    {
        free(ci->token[i].token);
        for(unsigned long a = 0; a < ci->token[i].subtoken_cnt; i++)
        {
            free(ci->token[i].subtoken[a]);
        }
        free(ci->token[i].subtoken);
    }
    free(ci->token);

    /* freeing labels */
    for(unsigned long i = 0; i < ci->label_cnt; i++)
    {
        free(ci->label[i].name);
    }
    free(ci->label);

    free(ci);
}

void compile_files(const char **files,
                   int file_cnt)
{
    /* allocating compiler invocation */
    compiler_invocation_t *ci = compiler_invocation_alloc();

    /* gathering code */
    get_code_buffer(files, file_cnt, ci);

    /*
     * formatting code by removing all the useless
     * and ignored stuff and extracting solely usable
     * information to process
     */
    code_remove_comments(ci);
    code_remove_newlines(ci);

    /* generating tokens,labels,sections out of the code */
    code_tokengen(ci);

    /* allocate space for the low level compiler to put resolved addresses at */
    code_token_label(ci);
    code_token_section(ci);
    code_token_macro(ci);

    /* finally compiling it to machine code */
    la64_compiler_lowlevel(ci);

    /* insert entry */
    code_token_label_insert_start(ci);

    /* spitting out binary */
    code_binary_spitout(ci);
}
