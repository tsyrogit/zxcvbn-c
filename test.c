/**********************************************************************************
 * Program to test the C implementation of the zxcvbn password strength estimator.
 * Copyright (c) 2015-2017 Tony Evans
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <zxcvbn.h>

/* For pre-compiled headers under windows */
#ifdef _WIN32
#include "stdafx.h"
#endif

const char *UsrDict[] =
{
    "Onename.Twoname@example.com", "Onename", "Twoname", "example.com", "example",
    0
};

static void CalcPass(const char *Pwd, int Quiet)
{
    double e;
    if (!Quiet)
    {
        /* Output the details of how the entropy figure was calculated */
        int Len, ChkLen;
        struct timeval t1, t2;
        ZxcMatch_t *Info, *p;
        double m = 0.0;

        gettimeofday(&t1, 0);
        e = ZxcvbnMatch(Pwd, UsrDict, &Info);
        gettimeofday(&t2, 0);
        for(p = Info; p; p = p->Next)
            m += p->Entrpy;

        Len = strlen(Pwd);
        m = e - m;
        printf("Pass %s \tLength %d\tEntropy bits=%.3f log10=%.3f\tMulti-word extra bits=%.1f\n", Pwd, Len, e, e * 0.301029996, m);
        p = Info;
        ChkLen = 0;
        while(p)
        {
            int n;
            switch((int)p->Type)
            {
                case BRUTE_MATCH:                     printf("  Type: Bruteforce     ");   break;
                case DICTIONARY_MATCH:                printf("  Type: Dictionary     ");   break;
                case DICT_LEET_MATCH:                 printf("  Type: Dict+Leet      ");   break;
                case USER_MATCH:                      printf("  Type: User Words     ");   break;
                case USER_LEET_MATCH:                 printf("  Type: User+Leet      ");   break;
                case REPEATS_MATCH:                   printf("  Type: Repeated       ");   break;
                case SEQUENCE_MATCH:                  printf("  Type: Sequence       ");   break;
                case SPATIAL_MATCH:                   printf("  Type: Spatial        ");   break;
                case DATE_MATCH:                      printf("  Type: Date           ");   break;
                case YEAR_MATCH:                      printf("  Type: Year           ");   break;
                case LONG_PWD_MATCH:                  printf("  Type: Extra-long     ");   break;
                case BRUTE_MATCH+MULTIPLE_MATCH:      printf("  Type: Bruteforce(Rep)");   break;
                case DICTIONARY_MATCH+MULTIPLE_MATCH: printf("  Type: Dictionary(Rep)");   break;
                case DICT_LEET_MATCH+MULTIPLE_MATCH:  printf("  Type: Dict+Leet(Rep) ");   break;
                case USER_MATCH+MULTIPLE_MATCH:       printf("  Type: User Words(Rep)");   break;
                case USER_LEET_MATCH+MULTIPLE_MATCH:  printf("  Type: User+Leet(Rep) ");   break;
                case REPEATS_MATCH+MULTIPLE_MATCH:    printf("  Type: Repeated(Rep)  ");   break;
                case SEQUENCE_MATCH+MULTIPLE_MATCH:   printf("  Type: Sequence(Rep)  ");   break;
                case SPATIAL_MATCH+MULTIPLE_MATCH:    printf("  Type: Spatial(Rep)   ");   break;
                case DATE_MATCH+MULTIPLE_MATCH:       printf("  Type: Date(Rep)      ");   break;
                case YEAR_MATCH+MULTIPLE_MATCH:       printf("  Type: Year(Rep)      ");   break;
                case LONG_PWD_MATCH+MULTIPLE_MATCH:   printf("  Type: Extra-long(Rep)");   break;

                default:                printf("  Type: Unknown%d ", p->Type);   break;
            }
            ChkLen += p->Length;
            printf("  Length %d  Entropy %6.3f (%.2f) ", p->Length, p->Entrpy, p->Entrpy * 0.301029996);
            for(n = 0; n < p->Length; ++n, ++Pwd)
                printf("%c", *Pwd);
            printf("\n");
            p = p->Next;
        }
        ZxcvbnFreeInfo(Info);
        t2.tv_sec -= t1.tv_sec;
        t2.tv_usec -= t1.tv_usec;
        t2.tv_usec += t2.tv_sec * 1000000;
        printf("    Calculation Time %.2fms\n", t2.tv_usec/1000.0);
        if (ChkLen != Len)
            printf("*** Password length (%d) != sum of length of parts (%d) ***\n", Len, ChkLen);
    }
    else
    {
        /* Only get the final entropy figure */
        e = ZxcvbnMatch(Pwd, UsrDict, 0);
        printf("Pass %s \tEntropy %.3f\n", Pwd, e);
    }
}

int DoChecks(char *file)
{
    char Line[5000];
    int y = 0;
    int w = 0;
    int r = 0;
    int Less = 0;
    int More = 0;
    FILE *f = fopen(file, "r");
    if (f == NULL)
    {
        printf("Failed to open %s\n", file);
        return 1;
    }
    memset(Line, 0, sizeof Line);
    while(fgets(Line, sizeof Line - 4, f))
    {
        /* Line is password + whitespace + expected entropy */
        char *Pwd, *s, *t;
        double Ent, e, x;
        unsigned int i;
        ++y;
        for(i = 0; i < sizeof Line - 5; ++i)
        {
            if (!Line[i] || (Line[i] == '\n'))
                break;
        }
        /* Skip blank lines or those starting with # */
        if ((i < 3) || (Line[0] == '#'))
            continue;
        memset(Line + i, 0, 4);
        Pwd = Line;
        /* Skip leading whitespace */
        while(*Pwd && (*Pwd <= ' '))
            ++Pwd;

        /* Make password null termnated */
        s = Pwd;
        t = strchr(s, '\t');
        if (t == NULL)
            t = strstr(s, "  ");
        if (t == NULL)
        {
            printf("Bad test condition on line %d\n", y);
            r = 1;
            break;
        }
        *t++ = 0;

        /* Skip whitespace before entropy value */
        while(*t && (*t <= ' '))
            ++t;
        if (!*t)
        {
            printf("Bad test condition on line %d\n", y);
            r = 1;
            break;
        }

        Ent = atof(t);
        if ((Ent < 0.0) || (Ent > 10000.0))
        {
            printf("Bad entropy value on line %d\n", y);
            r = 1;
            break;
        }
        e = ZxcvbnMatch(Pwd, UsrDict, 0);
        x = e / Ent;
        /* More than 1% difference is a fail. */
        if (x > 1.01)
        {
            ++More;
            if (r < 10)
            {
                printf("Line %2d Calculated entropy %5.2f, expected %5.2f  <%s>\n", y, e, Ent, Pwd);
                ++r;
            }
        }
        else if (x < 1.0/1.01)
        {
            ++Less;
            if (r < 10)
            {
                printf("Line %2d Calculated entropy %5.2f, expected %5.2f  <%s>\n", y, e, Ent, Pwd);
                ++r;
            }
        }
        ++w;
    }
    fclose(f);
    printf("Tested %d words, %d with low entropy, %d with high\n", w, Less, More);
    return r;
}

int main(int argc, char **argv)
{
    int i, Quiet, Checks, White;
    Quiet = 0;
    Checks = 0;
    White = 0;

    if (!ZxcvbnInit("zxcvbn.dict"))
    {
        printf("Failed to open dictionary file\n");
        return 1;
    }
    if ((argc > 1) && (argv[1][0] == '-'))
    {
        if (!strcmp(argv[1], "-qs") || !strcmp(argv[1], "-sq"))
            Quiet = White = 1;
        if (!strcmp(argv[1], "-t"))
            Checks = 1;
        if (!strcmp(argv[1], "-q"))
            Quiet = 1;
        if (!strcmp(argv[1], "-s"))
            White = 1;
        if ((Checks + Quiet + White) == 0)
        {
            char *s = strrchr(argv[0], '/');
            if (s == NULL)
                s = argv[0];
            else
                ++s;
            printf( "Usage: %s [ -q | -qs ] [ pwd1 pwd2 ... ]\n"
                    "          Output entropy of given passwords. If no passwords on command line read\n"
                    "           them from stdin.\n"
                    "          -q option stops password analysis details from being output.\n"
                    "          -s Ignore anything from space on a line when reading from stdin.\n"
                    "       %s -t file\n"
                    "          Read the file and check for correct results.\n", s, s);

            return 1;
        }
    }
    if (Checks)
    {
        for(i = 2; i < argc; ++i)
        {
            Checks = DoChecks(argv[i]);
            if (Checks)
                return 1;
        }
        return 0;
    }
    i = 1+Quiet+White;
    if (i >= argc)
    {
        /* No test passwords on command line, so get them from stdin */
        char Line[5000];
        while(fgets(Line, sizeof Line, stdin))
        {
            /* Drop the trailing newline character */
            for(i = 0; i < (int)(sizeof Line - 1); ++i)
            {
                if (Line[i] < ' ')
                {
                    Line[i] = 0;
                    break;
                }
                if (White && (Line[i] == ' '))
                {
                    Line[i] = 0;
                    break;
                }
            }
            if (Line[0])
                CalcPass(Line, Quiet);
        }
    }
    else
    {
        /* Do the test passwords on the command line */
        for(; i < argc; ++i)
        {
            CalcPass(argv[i], Quiet);
        }
    }
    ZxcvbnUnInit();
    return 0;
}
