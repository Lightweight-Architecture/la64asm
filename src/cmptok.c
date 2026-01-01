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
#include <la64asm/cmptok.h>

_Thread_local const char *ltokptr;
_Thread_local char otoken[255];

static void cmptok_skip_triggers(void)
{
    if(ltokptr[0] != ' ')
    {
        return;
    }

    ltokptr++;
    cmptok_skip_triggers();
}

const char *cmptok(const char *token)
{
    // If token is not null set ltokptr to the value of token
    if(token != NULL)
    {
        // Set ltokptr to token
        ltokptr = token;
    }
    else if(ltokptr == NULL && ltokptr[0] == '\0')
    {
        // NOTE: Return NULL because a nullified or nullterminated ltokptr is useless
        return NULL;
    }

    // Nullify otoken
    memset(otoken, 0, 255);

    // Copy token for token
    unsigned char a = 0;
    unsigned char token_mode = CMPTOK_TOKEN_MODE_NONE;
    while(a < 255)
    {
        // Check if its a space or a delimeter
        if(ltokptr[0] == ' ' && token_mode == CMPTOK_TOKEN_MODE_NONE)
        {
            cmptok_skip_triggers();
            break;
        }
        else if(ltokptr[0] == '"')
        {
            if(token_mode == CMPTOK_TOKEN_MODE_NONE)
            {
                token_mode = CMPTOK_TOKEN_MODE_DELIMETER;
            }
            else if(token_mode == CMPTOK_TOKEN_MODE_DELIMETER)
            {
                token_mode = CMPTOK_TOKEN_MODE_NONE;
            }
        }
        else if(ltokptr[0] == '\'')
        {
            if(token_mode == CMPTOK_TOKEN_MODE_NONE)
            {
                token_mode = CMPTOK_TOKEN_MODE_METER;
            }
            else if(token_mode == CMPTOK_TOKEN_MODE_METER)
            {
                token_mode = CMPTOK_TOKEN_MODE_NONE;
            }
        }
        else if(ltokptr[0] == ',')
        {
            if(token_mode == CMPTOK_TOKEN_MODE_NONE)
            {
                ltokptr++;
                continue;
            }
        }

        otoken[a] = ltokptr[0];

        // Check if ltokptr is nullterminated rn
        if(ltokptr[0] == '\0')
        {
            break;
        }

        // Incrementation
        a++;
        ltokptr++;
    }

    return (a == 0) ? NULL : otoken;
}
