/*
 * main.c: main function
 * $Id$
 *
 * Copyright: (c) 2004 Sam Hocevar <sam@zoy.org>
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the Do What The Fuck You Want To
 *   Public License as published by Banlu Kemiyatorn. See
 *   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>

#include "config.h"
#include "common.h"

#ifdef HAVE_GETOPT_LONG
#   define MOREINFO "Try `%s --help' for more information.\n"
#else
#   define MOREINFO "Try `%s -h' for more information.\n"
#endif

/* Used for the debug messages */
char *argv0 = NULL;
int debug = 1;

int main(int argc, char *argv[])
{
    char *mode = "auto";
    int c;
    int digit_optind = 0;

    argv0 = argv[0];

    for(;;)
    {
        int this_option_optind = optind ? optind : 1;
#ifdef HAVE_GETOPT_LONG
        int option_index = 0;
        static struct option long_options[] =
        {
            { "mode", 1, 0, 'm' },
            { "help", 0, 0, 'h' },
            { "quiet", 0, 0, 'q' },
            { "version", 0, 0, 'v' },
            { 0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv, "hm:qv", long_options, &option_index);
#else
        c = getopt(argc, argv, "hm:qv");
#endif
        if(c == -1)
            break;

        switch(c)
        {
        case 'h': /* --help */
            printf("Usage: %s [OPTION]... FILE...\n", argv[0]);
#ifdef HAVE_GETOPT_LONG
            printf("  -m, --mode      force operating mode\n");
            printf("  -q, --quiet     do not print information messages\n");
            printf("  -h, --help      display this help and exit\n");
            printf("  -v, --version   output version information and exit\n");
#else
            printf("  -m     force operating mode\n");
            printf("  -q     do not print information messages\n");
            printf("  -h     display this help and exit\n");
            printf("  -v     output version information and exit\n");
#endif
            return 0;
        case 'm': /* --mode */
            mode = optarg;
            break;
        case 'q': /* --quiet */
            debug = 0;
            break;
        case 'v': /* --version */
            printf("pwntcha (captcha decoder) %s\n", VERSION);
            printf("Written by Sam Hocevar.\n");
            printf("\n");
            printf("Copyright (C) 2004-2005 Sam Hocevar <sam@zoy.org>\n");
            printf("This is free software; see the source for copying conditions.  There is NO\n");
            printf("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
            return 0;
        case '?':
            printf(MOREINFO, argv[0]);
            return 1;
        default:
            printf("%s: invalid option -- %i\n", argv[0], c);
            printf(MOREINFO, argv[0]);
            return 1;
        }
    }

    if(optind >= argc)
    {
        printf("%s: too few arguments\n", argv[0]);
        printf(MOREINFO, argv[0]);
        return 1;
    }

    for(; optind < argc; optind++)
    {
        char *input = argv[optind], *result;
        struct image *img;

        img = image_load(argv[optind]);
        if(!img)
        {
            dprintf("cannot load %s\n", input);
            printf("\n");
            continue;
        }

        dprintf("image size %ix%i\n", img->width, img->height);

        if(!strcmp(mode, "test"))
            result = decode_test(img);
        else if(!strcmp(mode, "phpbb"))
            result = decode_phpbb(img);
        else if(!strcmp(mode, "scode"))
            result = decode_scode(img);
        else if(!strcmp(mode, "slashdot"))
            result = decode_slashdot(img);
        else
        {
            if(img->width == 320 && img->height == 50)
            {
                dprintf("autodetecting phpBB captcha\n");
                result = decode_phpbb(img);
            }
            else if(img->height == 25 || img->height == 30)
            {
                dprintf("autodetecting scode captcha\n");
                result = decode_scode(img);
            }
            else if(img->height == 69)
            {
                dprintf("autodetecting slashdot captcha\n");
                result = decode_slashdot(img);
            }
            else
            {
                dprintf("could not guess captcha type\n");
                printf("\n");
                image_free(img);
                continue;
            }
        }

        image_free(img);

        if(!result)
        {
            dprintf("sorry, decoding failed\n");
            printf("\n");
            continue;
        }

        printf("%s\n", result);
        free(result);
    }

    return 0;
}

void dprintf(const char *fmt, ...)
{
    va_list args;

    if(!debug)
        return;

    va_start(args, fmt);
    fprintf(stderr, "%s: ", argv0);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

