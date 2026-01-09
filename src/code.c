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

#include <la64asm/code.h>
#include <la64asm/cmptok.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void get_code_buffer(const char **files,
                     int file_cnt,
                     compiler_invocation_t *ci)
{
    /* allocating array for file descriptor */
    int *fd = calloc(file_cnt, sizeof(int));

    /* allocating array for file statistics */
    size_t *fdsize = calloc(file_cnt, sizeof(size_t));

    /* sizes */
    size_t size_needed = 0;
    size_t size_written = 0;

    /* calculating the total buffer size needed to store the code into */
    for(int i = 0; i < file_cnt; i++)
    {
        fd[i] = open(files[i], O_RDONLY);
        if(fd[i] < 0)
        {
            perror(files[i]);
            exit(EXIT_FAILURE);
        }

        struct stat fdstat;
        if(fstat(fd[i], &fdstat) < 0)
        {
            perror("fstat");
            exit(EXIT_FAILURE);
        }

        fdsize[i] = fdstat.st_size;
        size_needed += fdsize[i];
        size_needed++;
    }
    size_needed++;

    /* allocating buffer for the raw code */
    char *buf = malloc(size_needed + 1);

    /* null pointer check */
    if(buf == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /* read code into buffer*/
    for(int i = 0; i < file_cnt; i++)
    {
        /* initial read on file descriptor */
        ssize_t bytes = read(fd[i], buf + size_written, fdsize[i]);

        /* checking if bytes in the after math are below 0 */
        if(bytes < 0)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* nice write was successful */
        size_written += bytes;
        *((char*)(buf + size_written)) = '\n';
        size_written++;
    }

    /* null terminating string buffer */
    buf[size_written++] = '\n';             /* FIXME: Without this symbols break */
    buf[size_written] = '\0';

    /* cleaning the mess up, we dont need the file descriptors anymore */
    for(int i = 0; i < file_cnt; i++)
    {
        close(fd[i]);
    }

    /* releasing memory of the arrays we allocated */
    free(fd);
    free(fdsize);

    /* setting code buffer */
    ci->code = buf;
}

void code_tokengen(compiler_invocation_t *ci)
{
    /* gather information about the code */
    size_t token_cnt = 0;
    size_t code_size = strlen(ci->code);

    /* iterating through code and look for newline characters as indicator for a newline */
    for(size_t i = 0; i < code_size; i++)
    {
        if(ci->code[i] == '\n')
        {
            token_cnt += 1;
        }
    }

    /* allocate array of lines */
    ci->token = calloc(token_cnt, sizeof(compiler_token_t));
    ci->token_cnt = token_cnt;

    /* reset token count and then begin to copy */
    token_cnt = 0;
    size_t start_off = 0;   /* this offset is used to determine the lenght of each line */
    for(size_t i = 0; i < code_size; i++)
    {
        /* checking for new line character*/
        if(ci->code[i] == '\n')
        {
            size_t end_off = i;

            /* iteratting till hitting whitespace */
            for(; start_off < end_off; start_off++)
            {
                if(ci->code[start_off] != ' ')
                {
                    break;
                }
            }

            /* iteratting till hitting whitespace or a new line character */
            for(; end_off > start_off; end_off--)
            {
                if(ci->code[end_off] != ' ' && ci->code[end_off] != '\n')
                {
                    end_off++;
                    break;
                }
            }

            /* calculating the total lenght of the string */
            size_t len = end_off - start_off;

            /* allocating memory for the token and nullterminator  */
            ci->token[token_cnt].token = malloc(len + 1);

            /* copying line */
            memcpy(ci->token[token_cnt].token, &(ci->code[start_off]), len);

            /* nullterminating */
            ci->token[token_cnt].token[len] = '\0';

            start_off = i + 1;
            token_cnt++;
        }
    }

    /* getting subtokens of each token */
    for(unsigned long i = 0; i < ci->token_cnt; i++)
    {
        /* using cmptok in first pass to get token count */
        const char *token = cmptok(ci->token[i].token);
        while(token != NULL)
        {
            /* until this is not null i will not move anywhere else than my safe space which is this while loop :3*/
            ci->token[i].subtoken_cnt++;
            token = cmptok(NULL);
        }

        /* allocating memory for array of subtokens */
        ci->token[i].subtoken = calloc(sizeof(char*), ci->token[i].subtoken_cnt);

        /* copy subtokens */
        ci->token[i].subtoken_cnt = 0;

        /* again doing the same dance, over and over and over again, is this a carousell or why am I getting ill rn */
        token = cmptok(ci->token[i].token);
        while(token != NULL)
        {
            ci->token[i].subtoken[ci->token[i].subtoken_cnt++] = strdup(token);
            token = cmptok(NULL);
        }
    }

    /* token type evaluation */
    unsigned char section_mode = 0b0;
    for(unsigned long i = 0; i < ci->token_cnt; i++)
    {
        /* checking if valid token in the first place */
        if(ci->token[i].subtoken_cnt == 0)
        {
            continue;
        }

        /* lets go */
        if(ci->token[i].subtoken_cnt < 2)
        {
            /* getting size of subtoken */
            size_t size = strlen(ci->token[i].subtoken[0]);

            /* anti wrap around check */
            if(size == 0)
            {
                continue;
            }

            /* checking if last character of token is a ':', because that means that its a label */
            if(ci->token[i].subtoken[0][size - 1] == ':')
            {
                section_mode = 0b0;

                /* checking what type of label it is */
                if(ci->token[i].subtoken[0][0] == '_')
                {
                    ci->token[i].type = COMPILER_TOKEN_TYPE_LABEL;
                }
                else if(ci->token[i].subtoken[0][0] == '.')
                {
                    ci->token[i].type = COMPILER_TOKEN_TYPE_LABEL_IN_SCOPE;
                }
                else
                {
                    printf("[!] \"%s\" is not a legal label definition \n", ci->token[i].token);
                    exit(1);
                }

                continue;
            }
        }
        else if(ci->token[i].subtoken_cnt < 3)
        {
            /* checking if its a section */
            if(strcmp(ci->token[i].subtoken[0], "section") == 0)
            {
                section_mode = 0b1;
                ci->token[i].type = COMPILER_TOKEN_TYPE_SECTION;
                continue;
            }
        }
        else if(strcmp(ci->token[i].subtoken[0], "%define%") == 0)
        {
            section_mode = 0b0;

            ci->token[i].type = COMPILER_TOKEN_TYPE_SECTION_MACRODEF;
            continue;
        }

        /* last if else dance */
        if(section_mode)
        {
            /* its part of a section definition */
            ci->token[i].type = COMPILER_TOKEN_TYPE_SECTION_DATA;
        }
        else
        {
            /* its probably assembly code */
            ci->token[i].type = COMPILER_TOKEN_TYPE_ASM;
        }
    }
}

void code_binary_spitout(compiler_invocation_t *ci)
{
    /* open output file */
    int fd = open("a.out", O_RDWR | O_CREAT | O_TRUNC, 0666);

    /* writing output file */
    write(fd, ci->image, ci->image_addr);

    /* closing file descriptor */
    close(fd);
}