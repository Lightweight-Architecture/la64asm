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

void code_remove_comments(compiler_invocation_t *ci)
{
    size_t i = 0;
    while (ci->code[i] != '\0')
    {
        if (ci->code[i] == ';')
        {
            size_t s = i;
            while (s > 0 && ci->code[s - 1] == ' ')
            {
                s--;
            }
            if (s == 0 || ci->code[s - 1] == '\n')
            {
                i = s;
            }

            size_t e = i;
            while (ci->code[e] != '\n' && ci->code[e] != '\0')
            {
                e++;
            }

            memmove(&ci->code[i], &ci->code[e], strlen(&ci->code[e]) + 1);
            if (ci->code[i] == '\0')
            {
                break;
            }
        }
        else if (ci->code[i] == '/' && ci->code[i + 1] == '*')
        {
            size_t comment_start = i;  // Save BEFORE modifying i

            size_t s = i;
            while (s > 0 && ci->code[s - 1] == ' ')
            {
                s--;
            }
            if (s == 0 || ci->code[s - 1] == '\n')
            {
                i = s;
            }

            // Find closing */
            size_t e = comment_start + 2;
            while (ci->code[e] != '\0')
            {
                if (ci->code[e] == '*' && ci->code[e + 1] == '/')
                {
                    e += 2;
                    break;
                }
                e++;
            }

            // Skip trailing whitespace until newline
            while (ci->code[e] == ' ')
            {
                e++;
            }

            memmove(&ci->code[i], &ci->code[e], strlen(&ci->code[e]) + 1);
            if (ci->code[i] == '\0')
            {
                break;
            }
        }
        else
        {
            i++;
        }
    }
}

void code_remove_newlines(compiler_invocation_t *ci)
{
    size_t src = 0, dst = 0;
    bool last_was_newline = true;
    while (ci->code[src] != '\0')
    {
        if (ci->code[src] == '\r' || ci->code[src] == '\n') {
            if (!last_was_newline) {
                ci->code[dst++] = '\n';
                last_was_newline = true;
            }
            if (ci->code[src] == '\r' && ci->code[src + 1] == '\n') {
                src++;
            }
            src++;
            continue;
        }
        ci->code[dst++] = ci->code[src++];
        last_was_newline = false;
    }
    ci->code[dst] = '\0';
}

void code_replace_tab_with_spaces(compiler_invocation_t *ci)
{
    size_t src = 0;
    while(ci->code[src] != '\0')
    {
        if(ci->code[src] == '\t')
        {
            /* replacing with space */
            ci->code[src] = ' ';
        }
        src++;
    }
}

void code_tokengen(compiler_invocation_t *ci)
{
    /* Gathering token count */
    size_t token_cnt = 0;
    size_t code_size = strlen(ci->code);

    for(size_t i = 0; i < code_size; i++)
    {
        // Look for '\n'
        if(ci->code[i] == '\n')
        {
            token_cnt += 1;
        }
    }

    /* Gathering base tokens */
    ci->token = calloc(token_cnt, sizeof(compiler_token_t));
    ci->token_cnt = token_cnt;

    token_cnt = 0;
    size_t last_line_offset = 0;
    for(size_t i = 0; i < code_size; i++)
    {
        // Look for '\n'
        if(ci->code[i] == '\n')
        {
            size_t start_off = last_line_offset;
            size_t end_off = i;

            for(; start_off < end_off; start_off++)
            {
                if(ci->code[start_off] != ' ')
                {
                    break;
                }
            }

            for(; end_off > start_off; end_off--)
            {
                if(ci->code[end_off] != ' ' && ci->code[end_off] != '\n')
                {
                    end_off++;
                    break;
                }
            }

            size_t len = end_off - start_off;
            ci->token[token_cnt].token = malloc(len + 1);
            memcpy(ci->token[token_cnt].token, &(ci->code[start_off]), len);
            ci->token[token_cnt].token[len] = '\0';
            last_line_offset = i + 1;
            token_cnt++;
        }
    }

    /* Gathering their subtokens */
    for(unsigned long i = 0; i < ci->token_cnt; i++)
    {
        // Get amount of sub tokens
        ci->token[i].subtoken_cnt = 0;
        const char *token = cmptok(ci->token[i].token);
        while(token != NULL)
        {
            ci->token[i].subtoken_cnt++;
            token = cmptok(NULL);
        }

        // Allocate subtoken array
        ci->token[i].subtoken = calloc(sizeof(char*), ci->token[i].subtoken_cnt);

        // Get the actual subtokens
        ci->token[i].subtoken_cnt = 0;
        token = cmptok(ci->token[i].token);
        while(token != NULL)
        {
            ci->token[i].subtoken[ci->token[i].subtoken_cnt] = strdup(token);
            ci->token[i].subtoken_cnt++;
            token = cmptok(NULL);
        }
    }

    /* Evaluate Type */
    unsigned char section_mode = 0b0;
    for(unsigned long i = 0; i < ci->token_cnt; i++)
    {
        /* checking if valid token in the first place */
        if(ci->token[i].subtoken_cnt == 0)
        {
            continue;
        }

        if(ci->token[i].subtoken_cnt < 2)
        {
            // Label Check
            size_t size = strlen(ci->token[i].subtoken[0]);

            /* anti wrap around check */
            if(size == 0)
            {
                continue;
            }

            // Check if the last character of the first subtoken is a ':'
            if(ci->token[i].subtoken[0][size - 1] == ':')
            {
                section_mode = 0b0;

                // Its a scoped label if its first character is a dot
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
            if(strcmp(ci->token[i].subtoken[0], "section") == 0)
            {
                section_mode = 0b1;
                ci->token[i].type = COMPILER_TOKEN_TYPE_SECTION;
                continue;
            }
        }

        if(strcmp(ci->token[i].subtoken[0], "%macro%") == 0)
        {
            ci->token[i].type = COMPILER_TOKEN_TYPE_SECTION_MACRO;
            continue;
        }
        else if(strcmp(ci->token[i].subtoken[0], "%macroend%") == 0)
        {
            ci->token[i].type = COMPILER_TOKEN_TYPE_SECTION_MACROEND;
            continue;
        }
        else if(strcmp(ci->token[i].subtoken[0], "%define%") == 0)
        {
            ci->token[i].type = COMPILER_TOKEN_TYPE_SECTION_MACRODEF;
            continue;
        }
        
        if(section_mode)
        {
            // Its part of a section
            ci->token[i].type = COMPILER_TOKEN_TYPE_SECTION_DATA;
        }
        else
        {
            // Its probably ASM
            ci->token[i].type = COMPILER_TOKEN_TYPE_ASM;
        }
    }
}

void code_binary_spitout(compiler_invocation_t *ci)
{
    // Open a.out
    int fd = open("a.out", O_RDWR | O_CREAT | O_TRUNC, 0777);

    // Now write
    write(fd, ci->image, ci->image_addr);

    // Close file descriptor
    close(fd);
}

char *code_token_bind(compiler_token_t *ct, unsigned char at_i)
{
    /* null pointer check */
    if(ct == NULL || ct->subtoken_cnt < at_i)
    {
        return NULL;
    }

    /* getting size */
    unsigned short size = 0;
    for(unsigned char i = at_i; i < ct->subtoken_cnt; i++)
    {
        size += strlen(ct->subtoken[i]);
    }

    /* now try to alloc */
    char *name = calloc(1, size);
    char *ptr = name;

    /* doing shit */
    for(unsigned char i = at_i; i < ct->subtoken_cnt; i++)
    {
        size_t size = strlen(ct->subtoken[i]);
        memcpy(ptr, ct->subtoken[i], size);
        ptr += size;
    }

    return name;
}